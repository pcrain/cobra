#include "notifier.h"

namespace cobra {

const char* REMINDER_ICON = "document-edit";
const char* APP_NAME      = "Cobra";

int init_notify() { notify_init(APP_NAME); return 1; }
int end_notify()  { notify_uninit();       return 1; }

int show_notification(std::string summary, std::string title, std::string command, std::string iconpath) {
  return show_notification(-1,summary,title,command,iconpath);
}

int show_notification(int id, std::string summary, std::string title, std::string command, std::string iconpath) {

  const char* ipath = ((iconpath.length() == 0) ? REMINDER_ICON : iconpath.c_str());

  NotifyNotification* n = notify_notification_new(title.c_str(),summary.c_str(),ipath);
  // notify_notification_set_timeout(n, 10000); // 10 seconds
  notify_notification_set_urgency(n, NOTIFY_URGENCY_CRITICAL); // 10 seconds

  if (id >= 0) {  //Don't add actions for transient notifications
    std::string* spoint = new std::string(command);  //Free in handler
    std::string* mpoint = new std::string(summary);  //Free in handler
    noticeData* nd      = new noticeData;
    nd->index           = id;
    nd->commandptr      = spoint;
    nd->messageptr      = mpoint;
    void* castedData    = static_cast<void*>(nd);
    if(spoint->length() > 0) {
      notify_notification_add_action (
        n,"actionman","Handle & Dismiss",
        handleReminderCallback,castedData,handleReminderFree
      );
    } else {
      notify_notification_add_action (
        n,"nothingman","Dismiss",
        dismissCallback,castedData,dismissFree
      );
    }
    notify_notification_add_action (
      n,"snoozeman","Quick Snooze (60m)",
      quicksnoozeCallback,castedData,dismissFree
    );
    // notify_notification_add_action (
    //   n,"bigsnoozeman","Long Snooze (8h)",
    //   bigsnoozeCallback,castedData,dismissFree
    // );
    notify_notification_add_action (
      n,"customsnoozeman","Custom Snooze",
      customsnoozeCallback,castedData,dismissFree
    );

    //Add handler for closed event
    g_signal_connect(
      G_OBJECT (n),
      "closed",
      G_CALLBACK (closedCallback),
      castedData
    );
  }

  if (!notify_notification_show(n, 0)) {
    std::cerr << "show has failed" << std::endl;
    return -1;
  }
  return 0;
}

void closedCallback(NotifyNotification* notice, void* data) {
  // usleep(100);
  noticeData &nd = *(static_cast<noticeData*>(data));
  int id = nd.index;

  std::string s = "";
  char *empty = new char[s.length() + 1];
  strcpy(empty, s.c_str());

  unsigned reason = notify_notification_get_closed_reason(notice);
    // 1 - The notification expired.
    // 2 - The notification was dismissed by the user.
    // 3 - The notification was closed by a call to CloseNotification.
    // 4 - Undefined/reserved reasons.
  if ((not allAlarms[id].dismissed) && (allAlarms[id].remindTime < time(NULL))) {
    quicksnoozeCallback(notice,empty,data);
    show_notification("Reminder closed for 60 minutes!");
  }

  delete [] empty;
  saveAlarms(allAlarms,ALARM_DIR+ALARM_FILE);
}

void dismissCallback(NotifyNotification* notice, char* action, void* data) {
  noticeData &nd = *(static_cast<noticeData*>(data));
  allAlarms[nd.index].dismissed = true;
  allAlarms[nd.index].remindTime = time(NULL);
  allAlarms.erase(allAlarms.begin()+nd.index);
  saveAlarms(allAlarms,ALARM_DIR+ALARM_FILE);
}

void dismissFree(void* data) {
  //
  // show_notification("Hello world","some message text... bla bla","notify-send 'SUCCESS Man'");
}

void handleReminderCallback(NotifyNotification* notice, char* action, void* data) {
  // show_notification("Freeing Reminder "+std::to_string(nd.index),"","");
  noticeData &nd = *(static_cast<noticeData*>(data));
  const char* command = nd.commandptr->c_str();
  system(command);  //Execute the command
  dismissCallback(notice,action,data);
}

void handleReminderFree(void* data) {
  noticeData &nd = *(static_cast<noticeData*>(data));
  delete &(nd.commandptr);
  delete &(nd);
  saveAlarms(allAlarms,ALARM_DIR+ALARM_FILE);
}

void snoozeCallback(NotifyNotification* notice, char* action, void* data, int snoozeTime) {
  noticeData &nd           = *(static_cast<noticeData*>(data));
  int id                   = nd.index;
  allAlarms[id].remindTime = time(NULL)+snoozeTime;
  allAlarms[id].displayed  = false;
  saveAlarms(allAlarms,ALARM_DIR+ALARM_FILE);
}

void quicksnoozeCallback(NotifyNotification* notice, char* action, void* data) {
  snoozeCallback(notice,action,data,60*60);     //1 hour
}

void bigsnoozeCallback(NotifyNotification* notice, char* action, void* data) {
  snoozeCallback(notice,action,data,60*60*8);  //8 hours
}

void customsnoozeCallback(NotifyNotification* notice, char* action, void* data) {
  noticeData &nd = *(static_cast<noticeData*>(data));
  system(("/home/pretzel/scripts/notice-remind -c \""+(*nd.commandptr)+"\" "+(*nd.messageptr)).c_str()); // myfile.sh should be chmod +x
  dismissCallback(notice,action,data);
}

}
