/* -*- c++ -*-

This is Picprog, Microchip PIC programmer software for the serial port device.
Copyright © 1997,2002,2003,2004,2008,2010 Jaakko Hyvätti

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


#ifndef H_PICPORT
#define H_PICPORT

#include <ctime>

#include <termios.h>
#include <sys/ioctl.h>

class picport {
public:
  enum hardware_types {
    jdm, k8048
  };

private:
  int fd;
  struct termios saved, termstate;
  unsigned long addr;
  int debug_on;
  enum hardware_types hardware;
  char *portname;
  int W[16];

  int modembits;
  void set_clock_data (int rts, int dtr);
  void set_vpp (int vpp);

  void p_out (int b);
  int p_in ();

#if defined(__x86_64__) || defined(__i386__)
#define RDTSC_WORKS
#endif
// How many tsc clocks (cpu clock cycles) per 1000 ns
  static unsigned int tsc_1000ns;
  static int cable_delay;
  static int use_nanosleep;

public:

  static void delay (long ns);

  enum commands {
    load_conf = 0, data_for_prog = 2, data_from_prog = 4,
    inc_addr = 6, beg_prog = 010, data_for_data = 3,
    data_from_data = 5, erase_prog = 011, erase_data = 013,     
    command1 = 1, command7 = 7, end_prog = 016,
    end_prog_only = 027, beg_prog_only = 030, chip_erase = 037
  };

  enum commands18 {
    instr = 000,
    shift_out = 002,
    tread = 010,
    tread_inc = 011,
    tread_dec = 012,
    inc_tread = 013,
    twrite = 014,
    twrite_inc2 = 015,
    twrite_dec2 = 016,
    twrite_prog = 017,

    // flag for command18() method to implement programming delay by
    // holding fourth command clock cycle up.
    nop_prog = 0100,
    // Flag for erase delay
    nop_erase = 0200,
  };

  enum commands30 {
    SIX = 0,
    REGOUT = 1,
  };

  picport (const char *tty, bool nordtsc, bool slow, bool reboot,
	   enum hardware_types h);

  ~picport ();

  int command (enum commands comm, int data = 0);
  int command18 (enum commands18 comm, int data = 0);
  int command30 (enum commands30 comm, int data = 0);
  void setaddress (unsigned long a);
  void setaddress30 (unsigned long a);

  unsigned long address () { return addr; }

  void force ();
  void reset (unsigned long reset_address);
  const char *port () { return portname; }

  void debug (int d) { debug_on = d; }
};

#endif // H_PICPORT
