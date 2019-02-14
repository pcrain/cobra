#ifndef NOTIFIER_H_
#define NOTIFIER_H_

#include <iostream>            //std::cout

#include <libnotify/notify.h>  //NotifyNotification

#include "util.h"
#include "cobra.h"

namespace cobra {

class Alarm; //Forward declaration

int init_notify();
int end_notify();
int show_notification(int id, std::string summary, std::string title="Reminder", std::string command="", std::string iconpath="");
int show_notification(std::string summary, std::string title="Reminder", std::string command="", std::string iconpath="");

void closedCallback(NotifyNotification* notice, void* data);

void dismissCallback(NotifyNotification* notice, char* action, void* data);
void dismissFree(void* data);

void handleReminderCallback(NotifyNotification* notice, char* action, void* data);
void handleReminderFree(void* data);

void snoozeCallback(NotifyNotification* notice, char* action, void* data, int snoozeTime);
void quicksnoozeCallback(NotifyNotification* notice, char* action, void* data);
void bigsnoozeCallback(NotifyNotification* notice, char* action, void* data);
void customsnoozeCallback(NotifyNotification* notice, char* action, void* data);

struct noticeData {
  int index;
  std::string* commandptr;
  std::string* messageptr;
};

}

#endif /* NOTIFIER_H_ */
