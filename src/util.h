#ifndef UTIL_H_
#define UTIL_H_

#include <vector>   //std::vector
#include <fstream>  //ifstream, std::string
#include <cstdarg>  //va_start, va_list, va_end
#include <dirent.h> //DIR, opendir, readdir, closedir

namespace cobra {

const unsigned char  CHUNKSIZE  = 4;
const std::string    TIMEFORMAT = "%Y/%m/%d %H:%M:%S";

const std::string spaces[] = {
  "",
  " ",
  "  ",
  "   ",
  "    ",
  "     ",
  "      ",
  "       ",
  "        ",
  "         ",
  "          ",
  "           ",
  "            ",
  "             ",
  "              ",
  "               ",
  "                ",
  "                 ",
  "                  ",
  "                   ",
  "                    ",
  "                     ",
  "                      ",
  "                       ",
  "                        ",
  "                         ",
  "                          ",
  "                           ",
  "                            ",
  "                             ",
  "                              ",
  "                               ",
  "                                ",
  "                                 ",
  "                                  ",
};

// inline std::vector<std::string> getDirFiles(std::string dirname) {
//   std::vector<std::string> fvec;
//   DIR *dir;
//   struct dirent *ent;
//   if ((dir = opendir ("./alarms")) != NULL) {
//     while ((ent = readdir (dir)) != NULL) {
//       std::string fname = ent->d_name;
//       if (fname.compare(".") == 0)
//         continue;
//       if (fname.compare("..") == 0)
//         continue;
//       fvec.push_back(fname);
//     }
//     closedir (dir);
//   }
//   else {
//     perror ("");
//   }
//   return fvec;
// }

//http://stackoverflow.com/questions/69738/c-how-to-get-fprintf-results-as-a-stdstring-w-o-sprintf#69911
inline std::string vformat (const char *fmt, va_list ap) {
  // Allocate a buffer on the stack that's big enough for us almost
  // all the time.
  size_t size = 1024;
  char buf[size];

  // Try to vsnprintf into our buffer.
  va_list apcopy;
  va_copy (apcopy, ap);
  unsigned needed = vsnprintf (&buf[0], size, fmt, ap);
  // NB. On Windows, vsnprintf returns -1 if the string didn't fit the
  // buffer.  On Linux & OSX, it returns the length it would have needed.

  if (needed <= size && needed >= 0) {
      // It fit fine the first time, we're done.
      return std::string (&buf[0]);
  } else {
      // vsnprintf reported that it wanted to write more characters
      // than we allotted.  So do a malloc of the right size and try again.
      // This doesn't happen very often if we chose our initial size
      // well.
      std::vector <char> buf;
      size = needed;
      buf.resize (size);
      needed = vsnprintf (&buf[0], size, fmt, apcopy);
      return std::string (&buf[0]);
  }
}

//http://stackoverflow.com/questions/69738/c-how-to-get-fprintf-results-as-a-stdstring-w-o-sprintf#69911
inline std::string format (const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);
  std::string buf = vformat (fmt, ap);
  va_end (ap);
  return buf;
}

inline bool file_available(const char* fname) {
  std::ifstream infile(fname);
  return infile.good();
}

inline std::string intToFormattedTime(const long t, const std::string format = TIMEFORMAT) {
  struct tm * dt;
  char buffer [30];
  dt = localtime(&t);
  strftime(buffer, sizeof(buffer), format.c_str(), dt);
  return std::string(buffer);
}

inline long formattedTimeToInt(const std::string timestring, const std::string format = TIMEFORMAT) {
  struct tm tm;
  time_t t;
  if (strptime(timestring.c_str(), format.c_str(), &tm) == NULL) {/* Handle error */};
  tm.tm_isdst = -1; // tells mktime() to determine whether DST is in effect
  t = mktime(&tm);
  if (t == -1) {/* Handle error */};
  return t;
}

inline long timeToNow(const long t) {
  return difftime(t,time(NULL));
}

inline std::string padToLength(const std::string string, const unsigned length, const bool justifyRight = false, const char padchar = ' ') {
  std::string s = "";
  char p = padchar;
  if (string.length() <= length) {
    if (justifyRight) {
      // return (p*length+string)[-length:]
    }
    s.append(string);
    s.append(spaces[(length-string.length())]);
    return s;
  }
  s.append(string.substr(0,length-4));
  s.append("... ");
  return s;
}

inline std::string msFormat(const long intTime) {
  if (intTime >= 60) {
    int smin = (int)(intTime/60);
    int ssec = intTime-(60*smin);
    if (smin >= 60) {
      int shour = (int)(smin/60);
      smin = smin-(60*shour);
      if (shour >= 24) {
        int sday = (int)(shour/24);
        shour = shour-(24*sday);
        return std::to_string(sday)+"d"+std::to_string(shour)+"h"+std::to_string(smin)+"m"+std::to_string(ssec)+"s";
      }
      return std::to_string(shour)+"h"+std::to_string(smin)+"m"+std::to_string(ssec)+"s";
    }
    return std::to_string(smin)+"m"+std::to_string(ssec)+"s";
  }
  return std::to_string(intTime)+"s";
}

}

#endif /* UTIL_H_ */
