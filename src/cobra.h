#ifndef COBRA_H_
#define COBRA_H_

#include <regex>           //std::regex_match
#include <thread>          //std::thread
#include <signal.h>        //signal
#include <sys/file.h>      //flock
#include <sys/resource.h>  //setpriority
#include <pwd.h>           //getpwuid

#include <json/json.h>     //Json
#include <gtk/gtk.h>       //gtk_init, gtk_main

#include "termoutput.h"
#include "alarm.h"

namespace cobra {

  class Alarm; //Forward declaration

  const std::string        ALARM_DIR   = std::string(getpwuid(getuid())->pw_dir)+"/";
  const  std::string        ALARM_FILE  = ".cobra-alarms.json";
  const  std::string        ALARM_NEW   = ".cobra-alarms-new.json";
  const  int                RMLEN       = 30;     //length of reminder
  const  int                AMLEN       = 30;     //length of alarm command

  // const  std::string        ALARM_DIR   = "/home/pretzel/workspace/cobrac/alarms/";
  static bool               toUpdate    = true;   //whether we should update next frame
  static bool               quitOut     = false;  //whether we've hit 'q' to quit out
  static int                highlighted = -1;     //index of highlighted alarm

  extern std::vector<Alarm> allAlarms;

  void closeHandler(int signal);
  int checkIfAlreadyRunning();

  int run(int argc, char** argv);
  void gtkLoop(int* argc, char*** argv);
  void emptyLoop();
  void inputLoop();
  void outputLoop(bool daemonized=false);

  Json::Value loadJsonFromFile(std::string fname);
  std::vector<Alarm> loadAlarms(std::string path);
  void loadExtraAlarms(std::vector<Alarm>& alarms, std::string path);
  void saveAlarms(std::vector<Alarm> alarms, std::string fname);

}

#endif /* COBRA_H_ */
