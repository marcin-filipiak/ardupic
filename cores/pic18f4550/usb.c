/*
 * usb.c - USB CDC (virtual serial port) stack for PIC18F2550/4550 (SDCC).
 *
 * Polled USB engine, non ping-pong ("mode 0"). The SIE writes/reads the
 * Buffer Descriptor Table at 0x400; endpoint buffers live in USB RAM
 * (0x440-0x5FF) which the *_usb_g.lkr linker scripts reserve for __at().
 *
 * BD index = EP*2 + DIR (0x400 + 4*index), one BD per direction:
 *   bdt[0]=EP0 OUT, bdt[1]=EP0 IN, bdt[2]=EP1 OUT, bdt[3]=EP1 IN
 *
 * USB module interrupts are NOT enabled (PIE2<USBIE> kept 0, UIE kept 0);
 * the engine is serviced by usbcdc_poll() from main().
 *
 * Control-transfer data toggle: EP0 toggles are tracked in firmware
 * (ep0_dts) because a SETUP token is always DATA0 and does not follow the
 * alternating data-toggle rule. Every EP0 arm writes its expected toggle
 * explicitly and advances the counter.
 */
#include <Arduino.h>
#include <usb.h>

/* ---- PIC18F2550/4550 USB module bit masks (gputils p18f2550.inc) ---- */
#define UCON_USBEN    0x08u   /* UCON<3> */
#define UCON_PKTDIS   0x10u   /* UCON<4> */
#define UCON_PPBRST   0x40u   /* UCON<6> */

#define UCFG_FS_PULLUP 0x14u  /* 0x10=UPUEN | 0x04=FSEN, FSPD=00 (full speed) */

#define UEP_EPINEN    0x02u   /* UEPn<1> */
#define UEP_EPOUTEN   0x04u   /* UEPn<2> */
#define UEP_EPHSHK    0x10u   /* UEPn<4> */

#define USTAT_DIRIN   0x04u   /* USTAT<2>: 1 = IN */

#define UIR_URSTIF    0x01u
#define UIR_UERRIF    0x02u
#define UIR_TRNIF     0x08u
#define UIR_IDLEIF    0x10u
#define UIR_STALLIF   0x20u

#define BDS_UOWN      0x80u   /* USB owns buffer */
#define BDS_DTS       0x40u   /* expected data toggle (DATA1) */
#define BDS_DTSEN     0x08u
#define BDS_BSTALL    0x04u

#define PID_SETUP     0x0Du   /* (STAT & 0x3C) >> 2 */

/* ---- USB RAM map (0x400..0x5FF on the 18F2550) ---- */
#define EP0OUT_BUF    0x440u
#define EP0IN_BUF     0x480u
#define EP1OUT_BUF    0x500u
#define EP1IN_BUF     0x540u
#define EP2IN_BUF     0x580u

#define EP0_BD_OUT    0
#define EP0_BD_IN     1
#define EP1_BD_OUT    2
#define EP1_BD_IN     3

#define EP0_MAX_PACKET 64
#define EP1_MAX_PACKET 64
#define EP2_MAX_PACKET 8

#define RX_RING       128
#define TX_RING       128
#define RING_MASK     0x7Fu

/* ---- Buffer Descriptor Table at 0x400 (4 bytes / entry) ---- */
typedef volatile struct {
    unsigned char stat;
    unsigned char cnt;
    unsigned char adrl;
    unsigned char adrh;
} bdt_entry_t;

static bdt_entry_t __at(0x400) bdt[8];
static unsigned char __at(EP0OUT_BUF) ep0out[EP0_MAX_PACKET];
static unsigned char __at(EP0IN_BUF)  ep0in[EP0_MAX_PACKET];
static unsigned char __at(EP1OUT_BUF) ep1out[EP1_MAX_PACKET];
static unsigned char __at(EP1IN_BUF)  ep1in[EP1_MAX_PACKET];
static unsigned char __at(EP2IN_BUF)  ep2in[EP2_MAX_PACKET];

/* ---- device state ---- */
enum {
    USB_DETACHED = 0,
    USB_DEFAULT,
    USB_ADDRESS,
    USB_CONFIGURED
};
static unsigned char usb_state = USB_DETACHED;
static unsigned char addr_pending = 0xFF;

/* ---- control transfer state ---- */
enum {
    ST_IDLE = 0,
    ST_DATA_IN,
    ST_DATA_OUT,
    ST_STATUS_IN,
    ST_STATUS_OUT,
    ST_STALL
};
static unsigned char ctrl_stage = ST_IDLE;
static unsigned char ep0_dts;                 /* next expected EP0 toggle */

static unsigned char setup[8];                /* copy of SETUP packet */

static const unsigned char __code *ctrl_flash;  /* code-space source */
static unsigned char *ctrl_ram;                 /* RAM source / target */
static unsigned char ctrl_use_flash;
static unsigned int  ctrl_len;
static unsigned int  ctrl_done;

static unsigned char line_coding[7];          /* baud + format (CDC) */
static unsigned char ctrl_line_state;         /* SET_CONTROL_LINE_STATE */
static unsigned char st_vals[2];              /* GET_STATUS / _CONFIG / _INTERFACE */

/* ---- CDC data ring buffers (plain RAM) ---- */
static volatile unsigned char rxbuf[RX_RING];
static volatile unsigned char txbuf[TX_RING];
static volatile unsigned char rxhead, rxtail;
static volatile unsigned char txhead, txtail;

/* ---------------------------------------------------------------- */
/* EP0 arming - every arm writes the expected toggle explicitly     */
/* ---------------------------------------------------------------- */

static void usb_enable_packets(void)
{
    UCON &= (unsigned char)~UCON_PKTDIS;
}

/* back to SETUP-ready: SETUP token is always DATA0 */
static void arm_ep0_out_setup(void)
{
    ep0_dts = 0;
    bdt[EP0_BD_OUT].cnt  = EP0_MAX_PACKET;
    bdt[EP0_BD_OUT].adrl = (unsigned char)(EP0OUT_BUF & 0xFF);
    bdt[EP0_BD_OUT].adrh = (unsigned char)(EP0OUT_BUF >> 8);
    bdt[EP0_BD_OUT].stat = BDS_UOWN | BDS_DTSEN;      /* expect DATA0 */
    bdt[EP0_BD_IN].stat  = 0;                         /* CPU owns EP0 IN */
}

/* receive the next OUT data / status packet (toggle = ep0_dts) */
static void arm_ep0_out_next(void)
{
    bdt[EP0_BD_OUT].cnt  = EP0_MAX_PACKET;
    bdt[EP0_BD_OUT].adrl = (unsigned char)(EP0OUT_BUF & 0xFF);
    bdt[EP0_BD_OUT].adrh = (unsigned char)(EP0OUT_BUF >> 8);
    bdt[EP0_BD_OUT].stat = (unsigned char)(BDS_UOWN | BDS_DTSEN |
                                           (ep0_dts ? BDS_DTS : 0));
    ep0_dts ^= 1;
}

/* send IN packet of n bytes (toggle = ep0_dts) */
static void arm_ep0_in(unsigned char n)
{
    bdt[EP0_BD_IN].cnt  = n;
    bdt[EP0_BD_IN].adrl = (unsigned char)(EP0IN_BUF & 0xFF);
    bdt[EP0_BD_IN].adrh = (unsigned char)(EP0IN_BUF >> 8);
    bdt[EP0_BD_IN].stat = (unsigned char)(BDS_UOWN | BDS_DTSEN |
                                          (ep0_dts ? BDS_DTS : 0));
    ep0_dts ^= 1;
}

static void usb_stall(void)
{
    ctrl_stage = ST_STALL;
    bdt[EP0_BD_OUT].cnt  = EP0_MAX_PACKET;
    bdt[EP0_BD_OUT].adrl = (unsigned char)(EP0OUT_BUF & 0xFF);
    bdt[EP0_BD_OUT].adrh = (unsigned char)(EP0OUT_BUF >> 8);
    bdt[EP0_BD_OUT].stat = BDS_UOWN | BDS_BSTALL;
    bdt[EP0_BD_IN].stat  = BDS_UOWN | BDS_BSTALL;
}

/* ---------------------------------------------------------------- */
/* descriptors                                                       */
/* ---------------------------------------------------------------- */

static const unsigned char __code DeviceDesc[18] = {
    18, 1,
    0x00, 0x02,                 /* bcdUSB 2.00 */
    0x02, 0x00, 0x00,           /* CDC, subclass 0, proto 0 */
    64,                         /* bMaxPacketSize0 */
    0x09, 0x12,                 /* idVendor 0x1209 (pid.codes) */
    0x01, 0x00,                 /* idProduct 0x0001 */
    0x00, 0x01,                 /* bcdDevice 1.00 */
    1, 2, 3,                    /* iManufacturer, iProduct, iSerial */
    1                           /* bNumConfigurations */
};

static const unsigned char __code ConfigDesc[67] = {
    /* configuration */
    9, 2, 67, 0, 2, 1, 0, 0x80, 0x32,
    /* interface 0: CDC communications */
    9, 4, 0, 0, 1, 0x02, 0x02, 0x01, 0,
    /* header functional */
    5, 0x24, 0x00, 0x10, 0x01,
    /* call management */
    5, 0x24, 0x01, 0x00, 1,
    /* ACM */
    4, 0x24, 0x02, 0x02,
    /* union */
    5, 0x24, 0x06, 0, 1,
    /* EP2 IN interrupt (serial state notifications) */
    7, 5, 0x82, 3, 8, 0, 0xFF,
    /* interface 1: CDC data */
    9, 4, 1, 0, 2, 0x0A, 0x00, 0x00, 0,
    /* EP1 OUT bulk */
    7, 5, 0x01, 2, 64, 0, 0,
    /* EP1 IN bulk */
    7, 5, 0x81, 2, 64, 0, 0
};

static const unsigned char __code StrLang[4] = { 4, 3, 0x09, 0x04 };

/* "ardupic" */
static const unsigned char __code StrManuf[16] = {
    16, 3, 'a',0,'r',0,'d',0,'u',0,'p',0,'i',0,'c',0
};
/* "ardupic USB CDC" */
static const unsigned char __code StrProduct[32] = {
    32, 3,
    'a',0,'r',0,'d',0,'u',0,'p',0,'i',0,'c',0,' ',0,
    'U',0,'S',0,'B',0,' ',0,'C',0,'D',0,'C',0
};
/* "0001" */
static const unsigned char __code StrSerial[10] = {
    10, 3, '0',0,'0',0,'0',0,'1',0
};

/* ---------------------------------------------------------------- */
/* control transfer engine                                           */
/* ---------------------------------------------------------------- */

/* fill EP0 IN with the next chunk and arm it */
static void usb_send_chunk(void)
{
    unsigned char n = 0;
    while ((n < EP0_MAX_PACKET) && (ctrl_done < ctrl_len)) {
        if (ctrl_use_flash)
            ep0in[n] = ctrl_flash[ctrl_done];
        else
            ep0in[n] = ctrl_ram[ctrl_done];
        n++;
        ctrl_done++;
    }
    arm_ep0_in(n);
}

static void usb_start_in_code(const unsigned char __code *p,
                              unsigned int n, unsigned int wlen)
{
    ctrl_flash     = p;
    ctrl_ram       = 0;
    ctrl_use_flash = 1;
    ctrl_len       = (wlen < n) ? wlen : n;
    ctrl_done      = 0;
    ctrl_stage     = ST_DATA_IN;
    usb_send_chunk();
}

static void usb_start_in_ram(const unsigned char *p,
                             unsigned int n, unsigned int wlen)
{
    ctrl_ram       = (unsigned char *)p;
    ctrl_use_flash = 0;
    ctrl_len       = (wlen < n) ? wlen : n;
    ctrl_done      = 0;
    ctrl_stage     = ST_DATA_IN;
    usb_send_chunk();
}

/* start a data-OUT stage (host -> device, payload accumulates in <p>) */
static void usb_start_out_ram(unsigned char *p, unsigned int n)
{
    ctrl_ram       = p;
    ctrl_use_flash = 0;
    ctrl_len       = n;
    ctrl_done      = 0;
    ctrl_stage     = ST_DATA_OUT;
    arm_ep0_out_next();
}

static void usb_configure(unsigned char cfg)
{
    if (cfg == 1) {
        usb_state = USB_CONFIGURED;
        UEP1 = UEP_EPOUTEN | UEP_EPINEN | UEP_EPHSHK;  /* bulk IN+OUT */
        UEP2 = UEP_EPINEN  | UEP_EPHSHK;               /* intr IN */
        bdt[EP1_BD_OUT].cnt  = EP1_MAX_PACKET;
        bdt[EP1_BD_OUT].adrl = (unsigned char)(EP1OUT_BUF & 0xFF);
        bdt[EP1_BD_OUT].adrh = (unsigned char)(EP1OUT_BUF >> 8);
        bdt[EP1_BD_OUT].stat = BDS_UOWN | BDS_DTSEN;   /* expect DATA0 */
        bdt[EP1_BD_IN].stat  = 0;                      /* CPU owned */
    } else {
        usb_state = USB_ADDRESS;
        UEP1 = 0;
        UEP2 = 0;
        bdt[EP1_BD_OUT].stat = 0;
        bdt[EP1_BD_IN].stat  = 0;
        rxhead = rxtail = 0;
        txhead = txtail = 0;
    }
}

static void usb_handle_setup(void)
{
    unsigned char bm      = setup[0];
    unsigned char req     = setup[1];
    unsigned char wv      = setup[2];
    unsigned char wh      = setup[3];
    unsigned char wi      = setup[4];
    unsigned int  wlen    = (unsigned int)(setup[6] | ((unsigned int)setup[7] << 8));
    unsigned char type    = bm & 0x60u;
    unsigned char recip   = bm & 0x1Fu;
    unsigned char handled = 0;

    ctrl_len       = 0;
    ctrl_done      = 0;
    ctrl_use_flash = 0;
    ctrl_ram       = 0;
    ctrl_flash     = 0;

    /* first response after a SETUP (DATA0) is DATA1 */
    ep0_dts = 1;

    if (type == 0x00u) {                     /* standard request */
        switch (req) {
        case 0x05:                           /* SET_ADDRESS */
            addr_pending = wv;
            ctrl_stage = ST_STATUS_IN;
            arm_ep0_in(0);
            handled = 1;
            break;
        case 0x06:                           /* GET_DESCRIPTOR */
            switch (wh) {
            case 1:
                usb_start_in_code(DeviceDesc, sizeof(DeviceDesc), wlen);
                handled = 1;
                break;
            case 2:
                usb_start_in_code(ConfigDesc, sizeof(ConfigDesc), wlen);
                handled = 1;
                break;
            case 3:
                switch (wv) {
                case 0:
                    usb_start_in_code(StrLang, sizeof(StrLang), wlen);
                    handled = 1;
                    break;
                case 1:
                    usb_start_in_code(StrManuf, sizeof(StrManuf), wlen);
                    handled = 1;
                    break;
                case 2:
                    usb_start_in_code(StrProduct, sizeof(StrProduct), wlen);
                    handled = 1;
                    break;
                case 3:
                    usb_start_in_code(StrSerial, sizeof(StrSerial), wlen);
                    handled = 1;
                    break;
                default:
                    handled = 0;
                }
                break;
            default:
                handled = 0;                 /* incl. device qualifier */
            }
            break;
        case 0x00:                           /* GET_STATUS */
            st_vals[0] = 0;
            st_vals[1] = 0;
            usb_start_in_ram(st_vals, 2, wlen);
            handled = 1;
            break;
        case 0x08:                           /* GET_CONFIGURATION */
            st_vals[0] = (usb_state == USB_CONFIGURED) ? 1 : 0;
            usb_start_in_ram(st_vals, 1, wlen);
            handled = 1;
            break;
        case 0x0A:                           /* GET_INTERFACE */
            st_vals[0] = 0;
            usb_start_in_ram(st_vals, 1, wlen);
            handled = 1;
            break;
        case 0x09:                           /* SET_CONFIGURATION */
            usb_configure(wv);
            ctrl_stage = ST_STATUS_IN;
            arm_ep0_in(0);
            handled = 1;
            break;
        case 0x01:                           /* CLEAR_FEATURE */
        case 0x03:                           /* SET_FEATURE */
            ctrl_stage = ST_STATUS_IN;
            arm_ep0_in(0);
            handled = 1;
            break;
        case 0x0B:                           /* SET_INTERFACE */
            ctrl_stage = ST_STATUS_IN;
            arm_ep0_in(0);
            handled = 1;
            break;
        case 0x07:                           /* SET_DESCRIPTOR - not used */
        default:
            handled = 0;
        }
    } else if ((type == 0x20u) && (recip == 0x01u)) {  /* CDC class, interface */
        switch (req) {
        case 0x20:                           /* SET_LINE_CODING (7 bytes OUT) */
            usb_start_out_ram(line_coding, 7);
            handled = 1;
            break;
        case 0x21:                           /* GET_LINE_CODING (7 bytes IN) */
            usb_start_in_ram(line_coding, 7, wlen);
            handled = 1;
            break;
        case 0x22:                           /* SET_CONTROL_LINE_STATE */
            ctrl_line_state = wi;
            ctrl_stage = ST_STATUS_IN;
            arm_ep0_in(0);
            handled = 1;
            break;
        case 0x23:                           /* SEND_BREAK */
            ctrl_stage = ST_STATUS_IN;
            arm_ep0_in(0);
            handled = 1;
            break;
        default:
            handled = 0;
        }
    }

    if (!handled)
        usb_stall();
    usb_enable_packets();
}

/* ---------------------------------------------------------------- */
/* transaction dispatchers                                           */
/* ---------------------------------------------------------------- */

static void usb_ep0_out(void)
{
    unsigned char pid = (unsigned char)((bdt[EP0_BD_OUT].stat >> 2) & 0x0F);

    if (pid == PID_SETUP) {
        unsigned char i;
        for (i = 0; i < 8; i++)
            setup[i] = ep0out[i];
        ctrl_stage = ST_IDLE;
        usb_handle_setup();
        return;
    }

    if (ctrl_stage == ST_STATUS_OUT) {
        /* host status ZLP for an IN transfer -> done */
        ctrl_stage = ST_IDLE;
        arm_ep0_out_setup();
        return;
    }

    if (ctrl_stage == ST_DATA_OUT) {
        unsigned char n = bdt[EP0_BD_OUT].cnt;
        unsigned char i;
        if (ctrl_ram != 0) {
            for (i = 0; i < n; i++)
                ctrl_ram[ctrl_done + i] = ep0out[i];
        }
        ctrl_done += n;
        if (ctrl_done >= ctrl_len) {
            /* all payload in -> status IN ZLP (toggle tracked in ep0_dts) */
            ctrl_stage = ST_STATUS_IN;
            arm_ep0_in(0);
        } else {
            arm_ep0_out_next();
        }
        return;
    }

    /* stray OUT data: recover to idle */
    ctrl_stage = ST_IDLE;
    arm_ep0_out_setup();
}

static void usb_ep0_in(void)
{
    if (addr_pending != 0xFF) {
        UADDR = addr_pending;
        addr_pending = 0xFF;
        usb_state = (UADDR != 0) ? USB_ADDRESS : USB_DEFAULT;
    }

    switch (ctrl_stage) {
    case ST_DATA_IN:
        if (ctrl_done < ctrl_len) {
            usb_send_chunk();
        } else {
            ctrl_stage = ST_STATUS_OUT;      /* wait for host ZLP */
            arm_ep0_out_next();
        }
        break;
    case ST_STATUS_IN:
        ctrl_stage = ST_IDLE;
        arm_ep0_out_setup();
        break;
    default:
        ctrl_stage = ST_IDLE;
        arm_ep0_out_setup();
        break;
    }
}

static void usb_ep1_out(void)
{
    unsigned char n = bdt[EP1_BD_OUT].cnt;
    unsigned char i;
    unsigned char h = rxhead;

    for (i = 0; i < n; i++) {
        rxbuf[h] = ep1out[i];
        h = (unsigned char)(h + 1) & RING_MASK;
    }
    rxhead = h;

    bdt[EP1_BD_OUT].cnt  = EP1_MAX_PACKET;
    bdt[EP1_BD_OUT].stat = (unsigned char)(BDS_UOWN | BDS_DTSEN |
                                           (bdt[EP1_BD_OUT].stat & BDS_DTS));
}

static void usb_pump_tx(void);

static void usb_trn_handler(void)
{
    while (UIR & UIR_TRNIF) {
        unsigned char u   = USTAT;
        unsigned char ep  = (unsigned char)(u >> 3);
        unsigned char din = (unsigned char)(u & USTAT_DIRIN);

        if (ep == 0) {
            if (din)
                usb_ep0_in();
            else
                usb_ep0_out();
        } else if (ep == 1) {
            if (din)
                usb_pump_tx();
            else
                usb_ep1_out();
        }
        /* EP2 IN = serial state notifications: not armed, NAK'd */

        UIR &= (unsigned char)~UIR_TRNIF;
    }
}

static void usb_reset_handler(void)
{
    UADDR = 0;
    UIR   = 0;
    UEIR  = 0;
    UIE   = 0;                               /* polled engine: no USB IRQ */

    UEP0  = UEP_EPHSHK;                      /* control endpoint */
    UEP1  = 0;
    UEP2  = 0;

    usb_state    = USB_DEFAULT;
    addr_pending = 0xFF;
    ctrl_stage   = ST_IDLE;
    rxhead = rxtail = 0;
    txhead = txtail = 0;

    /* reset ping-pong pointers (harmless in mode 0) */
    UCON |= UCON_PPBRST;
    UCON &= (unsigned char)~UCON_PPBRST;

    arm_ep0_out_setup();
    usb_enable_packets();
}

/* ---------------------------------------------------------------- */
/* TX path                                                           */
/* ---------------------------------------------------------------- */

static unsigned char tx_queued(void)
{
    return (unsigned char)((txtail - txhead) & RING_MASK);
}

/* arm EP1 IN with up to one max-packet of queued data, if free */
static void usb_pump_tx(void)
{
    unsigned char n, i, t;

    if (usb_state != USB_CONFIGURED)
        return;
    if (bdt[EP1_BD_IN].stat & BDS_UOWN)
        return;                              /* IN pending */

    n = tx_queued();
    if (n == 0)
        return;
    if (n > EP1_MAX_PACKET)
        n = EP1_MAX_PACKET;

    t = txhead;
    for (i = 0; i < n; i++) {
        ep1in[i] = txbuf[t];
        t = (unsigned char)(t + 1) & RING_MASK;
    }
    txhead = t;

    bdt[EP1_BD_IN].cnt  = n;
    bdt[EP1_BD_IN].adrl = (unsigned char)(EP1IN_BUF & 0xFF);
    bdt[EP1_BD_IN].adrh = (unsigned char)(EP1IN_BUF >> 8);
    bdt[EP1_BD_IN].stat = (unsigned char)(BDS_UOWN | BDS_DTSEN |
                                          (bdt[EP1_BD_IN].stat & BDS_DTS));
}

/* ---------------------------------------------------------------- */
/* public API                                                        */
/* ---------------------------------------------------------------- */

void usbcdc_begin(void)
{
    /* disable any USB CPU interrupt path (we poll UIR instead) */
    PIE2 &= (unsigned char)~(0x20u);         /* clear USBIE (PIE2<5>) */
    UIE  = 0;
    UEIE = 0;
    UIR  = 0;

    UCON = 0;
    UCFG = UCFG_FS_PULLUP;
    UCON = UCON_USBEN;                       /* soft-attach */

    usb_state = USB_DEFAULT;

    /* default line coding: 9600 8N1 */
    line_coding[0] = 0x80;                   /* 0x2580 = 9600 baud */
    line_coding[1] = 0x25;
    line_coding[2] = 0x00;
    line_coding[3] = 0x00;
    line_coding[4] = 0x00;                   /* stop bits: 0 = 1 bit */
    line_coding[5] = 0x00;                   /* parity: none */
    line_coding[6] = 8;                      /* data bits */

    usbcdc_poll();
}

void usbcdc_poll(void)
{
    if (UIR & UIR_URSTIF)
        usb_reset_handler();
    if (UIR & UIR_TRNIF)
        usb_trn_handler();
    if (UIR & UIR_IDLEIF)
        UIR &= (unsigned char)~UIR_IDLEIF;   /* suspended: keep alive */
    if (UIR & UIR_UERRIF)
        UIR &= (unsigned char)~UIR_UERRIF;
    if (UIR & UIR_STALLIF)
        UIR &= (unsigned char)~UIR_STALLIF;

    /* flush queued TX whenever EP1 IN is free (arms the very first
       packet, which the completion path alone would never do) */
    if (usb_state == USB_CONFIGURED)
        usb_pump_tx();
}

void usbcdc_write(unsigned char c)
{
    unsigned long spins = 0;

    if (usb_state != USB_CONFIGURED)
        return;                              /* drop until enumerated */

    while (tx_queued() >= (TX_RING - 1)) {
        usbcdc_poll();
        if (++spins > 2000000UL)
            return;                          /* host stalled: drop byte */
    }
    txbuf[txtail] = c;
    txtail = (unsigned char)(txtail + 1) & RING_MASK;

    usbcdc_poll();                           /* prime EP1 IN if free */
}

int usbcdc_read(void)
{
    unsigned char c;
    if ((unsigned char)(rxhead - rxtail) == 0U)
        return -1;
    c = rxbuf[rxtail];
    rxtail = (unsigned char)(rxtail + 1) & RING_MASK;
    return (int)c;
}

int usbcdc_available(void)
{
    return (int)((unsigned char)(rxhead - rxtail) & RING_MASK);
}

void usbcdc_print(const char *s)
{
    while (*s)
        usbcdc_write((unsigned char)*s++);
}

void usbcdc_println(const char *s)
{
    usbcdc_print(s);
    usbcdc_write('\r');
    usbcdc_write('\n');
}

static void usbcdc_put_udec(unsigned long v)
{
    char buf[12];
    unsigned char i = 0;
    do {
        buf[i++] = (char)('0' + (v % 10UL));
        v /= 10UL;
    } while (v != 0UL);
    while (i)
        usbcdc_write((unsigned char)buf[--i]);
}

void usbcdc_print_ulong(unsigned long v)
{
    usbcdc_put_udec(v);
}

void usbcdc_print_int(long v)
{
    if (v < 0L) {
        usbcdc_write('-');
        usbcdc_put_udec((unsigned long)(-v));
    } else {
        usbcdc_put_udec((unsigned long)v);
    }
}