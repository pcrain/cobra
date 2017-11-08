#ifndef TERMOUTPUT_H_
#define TERMOUTPUT_H_

#include <curses.h>

#include "util.h"

namespace cobra {

static int _term_h, _term_w;
enum col { BLK,RED,GRN,YLW,BLU,MGN,CYN,WHT };

inline void init_curses() {
  initscr();                // Initialize the screen

  cbreak();                 // Don't buffer
  noecho();                 // Don't echo typed characters
  keypad(stdscr, TRUE);     // Get special keys
  wtimeout(stdscr,100);     // Block getch for at most 100ms

  start_color();            // Use colors
  attron(A_BOLD);           // All text is bold
  use_default_colors();     // Use default colors
  curs_set(0);              // Make cursor invisible

  for (int i = 0; i < 8; init_pair(i,i,-1),++i);
  getmaxyx(stdscr, _term_h, _term_w);
}

inline void end_curses() {
  curs_set(1);              // Make cursor visible again
  endwin();
}

inline void curprint(const char* format, ...) {
  attron(COLOR_PAIR(7));
  va_list argptr;
  va_start(argptr, format);
  addstr(vformat(format, argptr).c_str());
  va_end(argptr);
}

inline void curprint(const col color, const bool reverse, const char* format, ...) {
  attron(COLOR_PAIR(color));
  if (reverse) {
    attron(A_REVERSE);
  }
  va_list argptr;
  va_start(argptr, format);
  addstr(vformat(format, argptr).c_str());
  va_end(argptr);
  attroff(A_REVERSE);
}

}

#endif /* TERMOUTPUT_H_ */
