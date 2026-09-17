/* -*- c++ -*-

This is Testport, simple serial port testing program.
Copyright © 2007,2010 Jaakko Hyvätti

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

using namespace std;

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

int modem_signals[] = {
  TIOCM_CD,
  TIOCM_RI,
  TIOCM_CTS,
  TIOCM_DSR,
  TIOCM_RTS,
  TIOCM_DTR
};
const char * modem_signal_names[] = {
  "CD",
  "RI",
  "CTS",
  "DSR",
  "RTS",
  "DTR"
};
const int num_signals = sizeof(modem_signals) / sizeof(modem_signals[0]);

int
main (int argc, char **argv)
{
  int fd;
  struct termios saved, termstate, ttysaved, ttystate;
  int modembits;
  const char *portname;
  bool break_on = false;
  int signal_mask = 0;
  for (int s = 0; s < num_signals; ++s)
    signal_mask |= modem_signals[s];

  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " /dev/ttyS0" << endl;
    exit (1);
  }

  portname = argv[1];

  cout << "Keys:" << endl
       << "q\tquit" << endl
       << "b\ttoggle break" << endl
       << "r\ttoggle rts" << endl
       << "d\ttoggle dtr" << endl
       << "\tother keys transmit that char." << endl
    ;

  if (0 > (fd = open (portname, O_RDWR|O_NOCTTY|O_NONBLOCK))) {
    int e = errno;
    cerr << "Unable to open tty " << portname << ":" << strerror (e) << endl;
    exit (EX_IOERR);
  }
  tcgetattr (fd, &saved);
  termstate = saved;
  termstate.c_iflag = IGNBRK | IGNPAR;
  termstate.c_oflag = 0;
  termstate.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
  termstate.c_lflag = 0;
  termstate.c_cc[VMIN] = 0;
  termstate.c_cc[VTIME] = 0;
  tcsetattr (fd, TCSANOW, &termstate);

  ioctl (fd, TIOCMGET, &modembits);

  if (isatty (0)) {
    tcgetattr (0, &ttysaved);
    ttystate = ttysaved;
    ttystate.c_iflag = IGNBRK | IGNPAR;
    ttystate.c_oflag = OPOST | ONLCR;
    ttystate.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
    ttystate.c_lflag = 0;
    ttystate.c_cc[VMIN] = 0;
    ttystate.c_cc[VTIME] = 0;
    tcsetattr (0, TCSANOW, &ttystate);
    cout << "Terminal set up." << endl;
  }

  for (;;) {
    int n;
    char instr[2];
    n = read (0, instr, 1);
    if (n < 0) {
      int e = errno;
      cerr << "Read error on tty " << strerror (e) << endl;
      break;
    }
    if (n == 1) {
      if (instr[0] == 'q' || instr[0] == 3)
	break;
      if (instr[0] == 'b') {
	break_on = !break_on;
	if (0 > ioctl (fd, break_on ? TIOCSBRK : TIOCCBRK, 0)) {
	  int e = errno;
	  cerr << "Unable to set break on tty "
	       << portname << ":" << strerror (e) << endl;
	  break;
	}
	continue;
      }
      if (instr[0] == 'r') {
	int newmodem = modembits;
	newmodem ^= TIOCM_RTS;
	ioctl (fd, TIOCMSET, &newmodem);
	continue;
      }
      if (instr[0] == 'd') {
	int newmodem = modembits;
	newmodem ^= TIOCM_DTR;
	ioctl (fd, TIOCMSET, &newmodem);
	continue;
      }
      cout << "OUT: 0x" << hex << int(instr[0]) << dec << endl;
      if (1 != write (fd, instr, 1)) {
	cerr << "Write err" << strerror(errno) << endl;
	break;
      }
      continue;
    } // read 1 char from keyboard

    n = read (fd, instr, 1);
    if (n < 0) {
      int e = errno;
      cerr << "Read error on serial port " << strerror (e) << endl;
      break;
    }
    if (n == 1) {
      cout << "IN: 0x" << hex << int(instr[0]) << dec << endl;
    }

    int newmodem;
    ioctl (fd, TIOCMGET, &newmodem);
    if ((newmodem & signal_mask) != (modembits & signal_mask)) {
      string message;
      for (int s = 0; s < num_signals; ++s) {
	string sstate
	  = ((newmodem & modem_signals[s]) ? string("+") : string("-"))
	  + string(modem_signal_names[s]);
	cout << sstate << ' ';
	if ((newmodem & modem_signals[s]) != (modembits & modem_signals[s])) {
	  message += " " + sstate;
	}
      }
      cout << ':' << message << endl;
      modembits = newmodem;
    }

  } // for ever;

  if (isatty (0)) {
    tcsetattr (0, TCSANOW, &ttysaved);
  }
  tcsetattr (fd, TCSANOW, &saved);
  close (fd);
  cout << "Quit." << endl;
  return 0;
}
