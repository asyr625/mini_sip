#ifndef TERMMANIP_H
#define TERMMANIP_H

#ifdef COLOR_TERMINAL

#define FG_BLACK "\033[30m"
#define FG_BLUE "\033[34m"
#define FG_BROWN "\033[33m"
#define FG_GREEN "\033[32m"
#define FG_MAGENTA "\033[35m"
#define FG_RED "\033[31m"
#define FG_WHITE "\033[37m"
#define FG_CYAN "\033[36m"

#define BG_BLACK "\033[40m"
#define BG_BLUE "\033[44m"
#define BG_BROWN "\033[43m"
#define BG_CYAN "\033[46m"
#define BG_GREEN "\033[42m"
#define BG_MAGENTA "\033[45m"
#define BG_RED "\033[41m"
#define BG_WHITE "\033[47m"

#define BOLD "\033[2m\033[1m"
#define BREW "\033[2m\033[1m\033[7m"
#define PLAIN "\033[m"
#define FG_ERROR FG_RED

#else

#define BOLD ""
#define BREW ""
#define PLAIN ""
#define FG_ERROR ""

#define FG_BLACK ""
#define FG_BLUE ""
#define FG_BROWN ""
#define FG_GREEN ""
#define FG_MAGENTA ""
#define FG_RED ""
#define FG_WHITE ""
#define FG_CYAN ""

#define BG_BLACK ""
#define BG_BLUE ""
#define BG_BROWN ""
#define BG_CYAN ""
#define BG_GREEN ""
#define BG_MAGENTA ""
#define BG_RED ""
#define BG_WHITE ""

#endif

#endif // TERMMANIP_H
