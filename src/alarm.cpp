#include "alarm.h"

namespace cobra {

Alarm::Alarm(const Json::Value jroot) {
  this->j = jroot;

  this->reminder = this->j["reminder"].asString();

  std::string rt = this->j["remindTime"].asString();
  this->remindTime = formattedTimeToInt(rt);

  this->action = this->j["action"].asString();

  this->icon = this->j["icon"].asString();

  this->dismissed = this->j["dismissed"].asBool();

  _initAlarm();
}

Alarm::Alarm(const std::string reminder, const std::string timeString, const std::string action, const std::string icon) {
  Json::Value jroot;
  this->j = jroot;

  this->reminder = reminder;
  this->j["reminder"] = reminder;

  this->j["createTime"] = intToFormattedTime(time(NULL));

  this->remindTime = time(NULL)+_hmsTimeToInt(timeString);
  this->j["remindTime"] = intToFormattedTime(this->remindTime);

  this->action = action;
  this->j["action"] = action;

  this->icon = icon;
  this->j["icon"] = icon;

  this->dismissed = false;
  _initAlarm();
}

void Alarm::_initAlarm() {
  this->displayed = false;
  this->paddedReminder = padToLength(this->reminder,RMLEN);
  this->paddedAction = padToLength(this->action,RMLEN);
}

void Alarm::showReminder(const int id) const {
  show_notification(
    id,
    this->j["reminder"].asString(),
    "Reminder",
    this->j["action"].asString(),
    this->j["icon"].asString()
  );
}

void Alarm::updateData() {
  this->j["remindTime"] = intToFormattedTime(this->remindTime);
  this->j["dismissed"] = this->dismissed;
}

}
