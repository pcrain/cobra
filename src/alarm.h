#ifndef ALARM_H_
#define ALARM_H_

#include "util.h"
#include "notifier.h"

namespace cobra {

class Alarm {
  private:
    const char* HMS_REGEX = "^((?:[0-9]+)?)(h?)((?:[0-9]+)?)(m?)((?:[0-9]+)?)(s?)$";
    void _initAlarm();
    inline int _hmsTimeToInt(const std::string hms) const {
      std::cmatch m;
      std::regex_match(hms.c_str(), m, std::regex(HMS_REGEX));
      int time = 0, delta = 0;

      if (m[1].length() > 0) { delta = atoi(std::string(m[1]).c_str()); }
      if (m[2].length() > 0) { time += 60*60*delta; }
      if (m[3].length() > 0) { delta = atoi(std::string(m[3]).c_str()); }
      if (m[4].length() > 0) { time += 60*delta; }
      if (m[5].length() > 0) { delta = atoi(std::string(m[5]).c_str()); }
      if (m[6].length() > 0) { time += delta; }

      return time;
    }
  public:
    Json::Value j;
    std::string reminder;
    std::string action;
    std::string icon;
    std::string paddedReminder;
    std::string paddedAction;
    long        remindTime;
    bool        dismissed;
    bool        displayed;

    Alarm(const Json::Value jroot);
    Alarm(const std::string reminder, const std::string timeString, const std::string action = "", const std::string icon = "");
    void showReminder(const int id) const;
    void updateData();
};

struct alarmRemindTimeComparator {
  inline bool operator() (const Alarm& a1, const Alarm& a2) {
    if (a1.dismissed)
      return false;
    return (a2.dismissed || (a1.remindTime < a2.remindTime));
  }
};

}

#endif /* ALARM_H_ */
