/* -*- c++ -*-

This is Picprog, Microchip PIC programmer software for the serial port device.
Copyright © 1997,2002,2003,2004,2006,2007,2008,2010 Jaakko Hyvätti

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/ .

The author may be contacted at:

Email: Jaakko.Hyvatti@iki.fi
URL:   http://www.iki.fi/hyvatti/
Phone: +358 40 5011222

Please send any suggestions, bug reports, success stories etc. to the
Email address above.  Include word 'picprog' in the subject line to
make sure your email passes my spam filtering.

*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <ctime>

#include <sys/ioctl.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#include <sysexits.h>
#include <sched.h>

#include "picport.h"

using namespace std;

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

unsigned int picport::tsc_1000ns = 0;

// Data bitrate delay, ns.
// Cable delay is tested and this value adjusted when port is opened.
int picport::cable_delay = 0;

// Set this to -1 if you want to use nanosleep when running as root.
// This only works with 2.4 kernels, 2.6 series kernels no more work.
int picport::use_nanosleep = 0;

// 
static bool disable_interrupts = 1;

void
picport::set_clock_data (int rts, int dtr)
{
  if (hardware == k8048) {
    rts = !rts;
    dtr = !dtr;
  }
  // Before first call to set_clock_data, read the modem status like this:
  // ioctl (fd, TIOCMGET, &modembits);
  if (rts)
    modembits |= TIOCM_RTS;
  else
    modembits &= ~TIOCM_RTS;
  if (dtr)
    modembits |= TIOCM_DTR;
  else
    modembits &= ~TIOCM_DTR;
  if (0 > ioctl (fd, TIOCMSET, &modembits)) {
    int e = errno;
    tcsetattr (fd, TCSANOW, &saved);
    cerr << "Unable to set RTS/DTR on tty " << portname << ":" << strerror (e) << endl;
    exit (EX_IOERR);
  }
}

void
picport::set_vpp (int vpp)
{
  if (hardware == k8048)
    vpp = !vpp;
  if (0 > ioctl (fd, vpp ? TIOCSBRK : TIOCCBRK, 0)) {
    int e = errno;
    if (vpp)
      ioctl (fd, TIOCCBRK, 0);
    tcsetattr (fd, TCSANOW, &saved);
    cerr << "Unable to " << (vpp ? "start" : "stop") << " break on tty "
        << portname << ":" << strerror (e) << endl;
    exit (EX_IOERR);
  }
}

picport::picport (const char *tty, bool nordtsc, bool slow, bool reboot,
		  hardware_types h)
  : addr (0), debug_on (0), hardware (h)
{
  for (int i = 0; i < 16; ++i)
    W [i] = 0;
  portname = new char [strlen (tty) + 1];
  strcpy (portname, tty);

  if (0 > (fd = open (tty, O_RDWR|O_NOCTTY|O_NONBLOCK))) {
    int e = errno;
    cerr << "Unable to open tty " << tty << ":" << strerror (e) << endl;
    exit (EX_IOERR);
  }
  tcgetattr (fd, &saved);
  termstate = saved;
  termstate.c_iflag = IGNBRK | IGNPAR;
  termstate.c_oflag = 0;
  termstate.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
  termstate.c_lflag = 0;
  tcsetattr (fd, TCSANOW, &termstate);

  // Initialize lines for programming

  // RTS/DTR low, sleep and then TxD high (program voltage)

  ioctl (fd, TIOCCBRK, 0);
  // Before first call to set_clock_data, read the modem status.
  ioctl (fd, TIOCMGET, &modembits);
  modembits &= ~(TIOCM_RTS | TIOCM_DTR);
  ioctl (fd, TIOCMSET, &modembits);
  usleep (1000);
  // Check the CTS.  If it is up, even when we just lowered DTR,
  // we probably are not talking to a JDM type programmer.
  int i = 0;
  ioctl (fd, TIOCMGET, &i);
  if (i & TIOCM_CTS) {
    tcsetattr (fd, TCSANOW, &saved);
    cerr << tty << ":CTS is high, probably we are not connected to a programmer but a modem or terminal." << endl;
    exit (EX_IOERR);
  }

  struct sched_param scp;
  scp.sched_priority = sched_get_priority_min (SCHED_FIFO);
  // sched_get_priority_max()
  // scp.sched_priority = 50;
  cerr << "Trying realtime priority " << scp.sched_priority << endl;
  if (sched_setscheduler (0, SCHED_FIFO, &scp)) {
    cerr << "Cannot use real time scheduling: " << strerror(errno) << endl;
    // Not root.  Cannot use realtime scheduling.
    use_nanosleep = 0;
  }
  if (iopl (3))
    disable_interrupts = 0;

#ifdef CPU_SETSIZE
  // When computing the delay loops, we do not want the cpu's to change.
  // Pick the first cpu in the current allowed set.
  cpu_set_t new_mask;
  cpu_set_t cur_mask;
  CPU_ZERO(&cur_mask);
  CPU_ZERO(&new_mask);
  if (sched_getaffinity(0, sizeof (cur_mask), &cur_mask) < 0) {
    tcsetattr (fd, TCSANOW, &saved);
    cerr << "Getting affinity functions do not work." << endl;
    exit (EX_IOERR);
  }
  for (int cpuno = 0; cpuno < CPU_SETSIZE; ++cpuno)
    if (CPU_ISSET (cpuno, &cur_mask)) {
      CPU_SET (cpuno, &new_mask);
      sched_getaffinity (0, sizeof (new_mask), &new_mask);
      // We want to print informative line only if we really
      // have multiple cpu's.
      for (int cpuno1 = cpuno+1; cpuno1 < CPU_SETSIZE; ++cpuno1)
	if (CPU_ISSET (cpuno1, &cur_mask)) {
	  cerr << "Bound to CPU " << cpuno << endl;
	  break;
	}
    }
#endif // CPU_SETSIZE

#ifdef RDTSC_WORKS
  if (!nordtsc && !use_nanosleep && !tsc_1000ns) {
    // Read /proc/cpuinfo for clock speed, or if that fails, try our
    // own test.
    ifstream cpui ("/proc/cpuinfo");
    string w;
    unsigned int mhz = 1; // 1MHz is just a flag that tsc was found
    while (cpui) {
      cpui >> w;
      if ("MHz" == w) {
	unsigned int tmp;
	// Read just the integer part.  No need for the rest.
	cpui >> w >> tmp;
	// Update this when we have 100 GHz CPUs.
	if (tmp >= 4 && tmp <= 100000)
	  mhz = tmp;
      }
      if ("tsc" == w && mhz) {
	// tsc capability found.  Use it.
	tsc_1000ns = mhz;
	break;
      }
    }

    // If the /proc fs did not contain clock speed but indicated
    // tsc capability, test the approximate clock speed.
    if (1 == tsc_1000ns) {
      // Loop the test 20 times, and select the smallest count.
      for (int recount = 0; recount < 20; ++recount) {
	struct timeval tv1, tv2;
	unsigned long a1, d1, a2, d2;

	// Wait for when a microsecond changes
	gettimeofday (&tv2, 0);
	do {
	  gettimeofday (&tv1, 0);
	} while (tv2.tv_usec == tv1.tv_usec);
	asm volatile("rdtsc":"=a" (a1), "=d" (d1));
	tv1.tv_usec += 1000;
	if (tv1.tv_usec >= 1000000) {
	  tv1.tv_usec -= 1000000;
	  tv1.tv_sec ++;
	}
	do {
	  gettimeofday (&tv2, 0);
	} while (tv2.tv_sec < tv1.tv_sec
		 || (tv2.tv_sec == tv1.tv_sec && tv2.tv_usec < tv1.tv_usec));
	asm volatile("rdtsc":"=a" (a2), "=d" (d2));
	if (a2 < a1)
	  d2 --;
	a2 -= a1;
	a2 &= 0xffffffff; // for x86_64
	d2 -= d1;
	if (d2)
	  continue;
	a2 /= 1000;
	if (tsc_1000ns <= 1 || tsc_1000ns > a2)
	  tsc_1000ns = a2;
      }
      if (tsc_1000ns <= 1) {
	tcsetattr (fd, TCSANOW, &saved);
	cerr << "Unable to determine CPU clock speed for delay loops." << endl;
	exit (EX_IOERR);
      }
    }
    if (tsc_1000ns >= 1) {
      cout << "CPU clock speed: " << tsc_1000ns << " MHz" << endl;
    }
  }
#endif

  if (tsc_1000ns > 1 && disable_interrupts)
    cerr << "Disable interrupts during delays." << endl;

  if (reboot) {
    // Power off any microcontroller that may be running a program.
    cerr << "Power off." << endl;
    set_clock_data (1, 1);
    set_vpp (1);
    usleep (500000); // 0.5s delay should discharge Vdd.
  }

  // /MCLR must go down for a while first
  set_vpp (0);
  usleep (10);
  // Power up
  set_clock_data (0, 0);
  set_vpp (1);
  // Charge Vdd
  usleep (25000);

  // Detect delays that are needed for cable lenght
  bool successful_echo = false;
  for (int i = 1; i <= 100; ++i) {
    int data_out = i & 1;
    // toggle data, keep clock off / power on
    if (data_out)
      modembits |= TIOCM_DTR;
    else
      modembits &= ~TIOCM_DTR;
    ioctl (fd, TIOCMSET, &modembits);
    delay (cable_delay);
    int data_in;
    ioctl (fd, TIOCMGET, &data_in);
    data_in = 0 != (data_in & TIOCM_CTS);
    delay (1000000);
    successful_echo = data_out == data_in;
    if (!successful_echo)
      cable_delay += 100; // Another 100 ns added for cable length
    ioctl (fd, TIOCMGET, &data_in);
    data_in = 0 != (data_in & TIOCM_CTS);
    if (data_out != data_in) {
      ioctl (fd, TIOCCBRK, 0);
      tcsetattr (fd, TCSANOW, &saved);
      cerr << "No JDM compatible hardware detected." << endl;
      exit (EX_IOERR);
    }
  }
  set_clock_data (0, 0);

  if (!successful_echo) {
    ioctl (fd, TIOCCBRK, 0);
    tcsetattr (fd, TCSANOW, &saved);
    cerr << "Cable too long." << endl;
    exit (EX_IOERR);
  }

  cable_delay += 100; // Another 100 ns added just for safety
  if (slow && cable_delay < 10000)
    cable_delay = 10000;
  if (1 == use_nanosleep)
    cerr << "Using " << cable_delay << " ns delays with real time priority." << endl;
  else if (tsc_1000ns > 1)
    cerr << "Using " << cable_delay << " ns delays." << endl;
  else if (cable_delay > 1000)
    cerr << "Using " << ((cable_delay + 999) / 1000) << " µs delays." << endl;
  else
    cerr << "Using >1 µs delays. --rdtsc may work for faster timings." << endl;
}

picport::~picport ()
{
  set_vpp (0);
  usleep (1);
  tcsetattr (fd, TCSANOW, &saved);
  close (fd);
  delete [] portname;
}

void picport::reset (unsigned long reset_address)
{
  set_clock_data (0, 0);
  usleep (100); // Make sure we have the power there.
  set_vpp (0);
  usleep (50);
  set_vpp (1);
  usleep (10);
  addr = reset_address;
}

void picport::delay (long ns)
{
  if (1 == use_nanosleep) {
    timespec ts = {ns / 1000000000, ns % 1000000000}, ts2;
    while (nanosleep (&ts, &ts2) && EINTR == errno)
      ts = ts2;
    return;
  }

#ifdef RDTSC_WORKS
  if (tsc_1000ns > 1) {
    unsigned long a1, d1, a2, d2;
    if (ns <= 10000 && disable_interrupts)
      asm volatile("pushf; cli");
    asm volatile("rdtsc":"=a" (a1), "=d" (d1));
    d2 = d1;
    if (ns > 10000)
      // This is not as accurate but does not overflow
      a2 = 0xffffffff & (a1 + 1 + (ns+999)/1000*tsc_1000ns);
    else
      a2 = 0xffffffff & (a1 + 1 + (ns*tsc_1000ns + 999) / 1000);
    if (a2 < a1)
      d2++;
    do {
      asm volatile("rdtsc":"=a" (a1), "=d" (d1));
    } while (d1 < d2 || (d1 == d2 && a1 < a2));
    if (ns <= 10000 && disable_interrupts)
      asm volatile("popf");
    return;
  }
  // Fall back to gettimeofday() if tsc is not available.
#endif // RDTSC_WORKS

  // Delay loop that should take more than a microsecond to execute.
  // Check the real time clock and break out if at least the specified
  // number of microseconds has gone.

  struct timeval tv1, tv2;
  gettimeofday (&tv2, 0);
  tv2.tv_usec += 1 + (ns + 999)/1000;
  if (tv2.tv_usec >= 1000000) {
    tv2.tv_usec -= 1000000;
    tv2.tv_sec++;
  }
  do {
    gettimeofday (&tv1, 0);
  } while (tv1.tv_sec < tv2.tv_sec
	   || (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec));
}

void picport::p_out (int b)
{
  struct timeval tv1, tv2;
  gettimeofday (&tv1, 0);
  if (tsc_1000ns > 1 && disable_interrupts)
    asm volatile("pushf; cli");
  set_clock_data (1, b); // set data, clock up
  delay (cable_delay);
  set_clock_data (0, b); // clock down
  if (tsc_1000ns > 1 && disable_interrupts)
    asm volatile("popf");
  gettimeofday (&tv2, 0);

  // We may have spent a long time in an interrupt or in another task
  // with clock down, so the pic may be pretty drained for power now.
  // Recharge at least as long as we were without power.
  long matching_delay = (tv2.tv_sec-tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
  if (matching_delay > 10) // 10 µs should be enough to charge up
    matching_delay = 10;
  matching_delay *= 1000;
  if (matching_delay < cable_delay)
    matching_delay = cable_delay;
  delay (matching_delay);
}

int picport::p_in ()
{
  struct timeval tv1, tv2;
  gettimeofday (&tv1, 0);
  if (tsc_1000ns > 1 && disable_interrupts)
    asm volatile("pushf; cli");
  set_clock_data (1, 1); // clock up
  delay (cable_delay);
  set_clock_data (0, 1); // set data up, clock down
  if (tsc_1000ns > 1 && disable_interrupts)
    asm volatile("popf");
  gettimeofday (&tv2, 0);

  // We may have spent a long time in an interrupt or in another task
  // with clock down, so the pic may be pretty drained for power now.
  // Recharge at least as long as we were without power.
  long matching_delay = (tv2.tv_sec-tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
  if (matching_delay > 10) // 10 µs should be enough to charge up
    matching_delay = 10;
  matching_delay *= 1000;
  if (matching_delay < cable_delay)
    matching_delay = cable_delay;
  delay (matching_delay);

  int i;

  ioctl (fd, TIOCMGET, &i);
  i = 0 != (i & TIOCM_CTS);
  if (hardware == k8048)
    return !i;
  return i;
}

int picport::command18 (enum commands18 comm, int data)
{
  int i, shift = comm;
  if (nop_prog == comm) {
    // A programming command must leave the last bit clock pulse up
    p_out (0);
    p_out (0);
    p_out (0);
    set_clock_data (1, 0); // clock up
    delay (1000*1000); // P9 >1 ms programming time
    set_clock_data (0, 0); // clock down
    // P10 >5 µs high voltage discharge time
    // Later models listed as > 100 µs
    delay (100*1000);
  } else {
    for (i = 0; i < 4; i++)
      p_out ((shift >> i) & 1);
    set_clock_data (0, 0); // set data down
    delay (1000);
  }

  shift = 0; // default return value

  switch (comm) {
  case nop_erase:
    // Erase cycle has delay between command and data
    usleep (10000); // P11 5 ms + P10 5 µs erase time
    // FALLTHROUGH

  case instr:
  case nop_prog:
    if (0x0e00 == (data & 0xff00))
      W[0] = data & 0x00ff;
    else if (0x6ef8 == data)
      addr = (addr & 0x00ffff) | (W[0] << 16);
    else if (0x6ef7 == data)
      addr = (addr & 0xff00ff) | (W[0] << 8);
    else if (0x6ef6 == data)
      addr = (addr & 0xffff00) | W[0];
    goto sw;

  case twrite_dec2:
    addr -= 2;
    goto sw;

  case twrite_inc2:
    addr += 2;
    // FALLTHROUGH

  case twrite:
  case twrite_prog:
  sw:
    for (i = 0; i < 16; i++)
      p_out ((data >> i) & 1);
    set_clock_data (0, 0); // set data down
    break;

  case tread_dec:
    --addr;
    goto sr;

  case tread_inc:
  case inc_tread:
    ++addr;
    // FALLTHROUGH

  case shift_out:
  case tread:
  sr:
    for (i = 0; i < 8; i++)
      p_out(0);
    delay (1000);
    for (i = 0; i < 8; i++)
      shift |= p_in () << i;
    set_clock_data (0, 0); // set data down
  }

  delay (1000);
  return shift;
}

int picport::command30 (enum commands30 comm, int data)
{
  int i, shift = comm;
  for (i = 0; i < 4; i++)
    p_out ((shift >> i) & 1);

  shift = 0; // default return value

  switch (comm) {
  case SIX:

    if (0x200000 == (data & 0xff0000))
      W[data & 15] = (data & 0x0ffff0) >> 4;
    else if (0xEB0000 == (data & 0xfff87f))
      W[(data >> 7) & 15] = 0;
    else if (0xBA1830 == (data & 0xfff870)) {
      // TBLRDL
      ++W[(data >> 7) & 15];
      ++W[data & 15];
    } else if (0xBA0830 == (data & 0xfff870)) {
      // TBLRDL
      ++W[(data >> 7) & 15];
      ++W[data & 15];
    } else if (0x880190 == (data & 0xfffff0))
      addr = (W[data & 15] << 16) & 0xff0000;
    addr = (addr & 0xff0000) | W[6];

    for (i = 0; i < 24; i++)
      p_out ((data >> i) & 1);
    break;

  case REGOUT:
    for (i = 0; i < 8; i++)
      p_out(0);
    for (i = 0; i < 16; i++)
      shift |= p_in () << i;
  }
  set_clock_data (0, 0); // set data down

  return shift;
}

void picport::setaddress (unsigned long a)
{
  if (0 != a && addr == a)
    return;

  command18 (instr, 0x0e00 | ((a & 0xff0000) >> 16));
  command18 (instr, 0x6ef8);
  command18 (instr, 0x0e00 | ((a & 0x00ff00) >> 8));
  command18 (instr, 0x6ef7);
  command18 (instr, 0x0e00 | (a & 0x0000ff));
  command18 (instr, 0x6ef6);
}

void picport::setaddress30 (unsigned long a)
{
  if (0 != a && addr == a)
    return;

  command30 (SIX, 0x200000 | ((a & 0xff0000) >> 12)); // MOV #, W0
  command30 (SIX, 0x880190); // MOV W0, TBLPAG
  command30 (SIX, 0x200006 | ((a & 0x00ffff) << 4)); // MOV #, W6
}

// -1 == error, no programmer present

int picport::command (enum commands comm, int data)
{
  int tmp1, tmp2;

  // first, send out the command, 6 bits

  int i, shift = comm;
  for (i = 0; i < 6; i++)
    p_out ((shift >> i) & 1);
  set_clock_data (0, 0); // set data down

  shift = 0; // default return value

  switch (comm) {
  case inc_addr:
    ++addr;
    if (data != 0) { // 12f508 and 12f509
      if (addr >= (unsigned long)(data))
	addr = 0;
      break;
    }

    if (addr >= 0x4000)
      addr = 0x2000;
    break;

  case data_from_prog:
  case data_from_data:
    delay (1000);
    tmp1 = p_in ();
    for (i = 0; i < 14; i++)
      shift |= p_in () << i;
    tmp2 = p_in ();
    set_clock_data (0, 0); // set data down

#ifdef CHECK_START_STOP
    // Start and stop bits must be 1.  Most of the old chips at least
    // conform to this test, but apparently pic12f635 and some other
    // later chips do not.

    if (!tmp1 || !tmp2) {
      cerr << portname << ":PIC programmer missing or chip fault" << endl;
      return -1;
    }
#endif

    if (data_from_data == comm) {

      // Check that the leftover bits were valid, all 1's.
      // This detects if the programmer is not connected to the port.
      // Unfortunately later chips clear these bits, so we must
      // accept both all 1's and all 0's.

      if ((shift & 0x3f00) != 0x3f00
	  && (shift & 0x3f00) != 0x0000) {
	cerr << portname << ": read value "
	     << hex << setfill('0') << setw(4) << shift << dec
	     << ": PIC programmer or chip fault\n"
	  "Is code protection enabled?  "
	  "Use --erase option to disable code protection." << endl;
	return -1;
      }

      shift &= 0xff;
    }
    break;

  case load_conf:
    addr = 0x2000;
    // FALLTHROUGH

  case data_for_prog:
  case data_for_data:
    delay (1000);
    p_out (0);
    for (i = 0; i < 14; i++)
      p_out ((data >> i) & 1);
    p_out (0);
    set_clock_data (0, 0); // set data down
    break;

  default:
    ;
  }

  delay (1000);
  return shift;
}

