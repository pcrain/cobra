#include "cobra.h"

namespace cobra {

  std::vector<Alarm> allAlarms;

  void closeHandler (int s) {
    if (s == SIGINT) {
      if (not quitOut) {
        signal(SIGINT, closeHandler);
        quitOut = true;
        std::cout << "Quitting out cleanly" << std::endl;
      } else {
        std::cout << "Quitting out violently" << std::endl;
      }
    } else if (s == SIGUSR1) {
      std::cout << "Update Signal Received" << std::endl;
      loadExtraAlarms(allAlarms,ALARM_DIR+ALARM_NEW);
    }
    // saveAlarms(allAlarms);
  }

  int checkIfAlreadyRunning() {
    std::string pfname = "/tmp/cobra.pid";
    // std::string pfname = (ALARM_DIR+std::string("cobra.pid")).c_str();

    int pid_file = open(pfname.c_str(), O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    uint32_t pid;
    if(rc) {
      if(EWOULDBLOCK == errno) {
        std::ifstream is;
        is.open(pfname.c_str());
        is.read(reinterpret_cast<char *>(&pid), sizeof(pid));
        std::cout << "Alarm daemon is already running! " << pid << std::endl;
        return pid;  //Bow out if we're running
      }
    } else {  //write and lock the file for ourself
      uint32_t pid = getpid();
      write(pid_file, &pid, sizeof(pid));
    }
    return -1;
  }

  int run(int argc, char** argv) {
    //Set up signal handlers
    signal(SIGINT, closeHandler);
    signal(SIGUSR1, closeHandler);

    //Set niceness
    setpriority(PRIO_PROCESS, getpid(), 5);

    bool daemonized = false;
    bool addAlarm = false;
    bool noinput = false;

    std::string newString = "(Generic Reminder)";
    std::string newTime = "60m";  //Defaults to 1 hour
    std::string newAction = "";
    std::string newIcon = "";

    int opt;
    opterr = 0; // Shut GetOpt error messages down (return '?')
    while ( (opt = getopt(argc, argv, "r:a:i:t:dn")) != -1 ) {
      switch ( opt ) {
        case 'r':
          newString = optarg;
          addAlarm = true; break;
        case 't':
          newTime = optarg;
          addAlarm = true; break;
        case 'a':
          newAction = optarg;
          addAlarm = true; break;
        case 'i':
          newIcon = optarg;
          addAlarm = true; break;
        case 'n':
          noinput = true;
          break;
        case 'd':
          daemonized = true;
          break;
        case '?':  // unknown option...
          std::cerr << "Unknown option: '" << char(optopt) << "'!" << std::endl;
          return -1;
      }
    }

    if (addAlarm) {
      std::vector<Alarm> newalarms;
      newalarms.push_back(Alarm(newString,newTime,newAction,newIcon));
      saveAlarms(newalarms,ALARM_DIR+ALARM_NEW);
    }

    int alreadyRunning = checkIfAlreadyRunning();
    if (alreadyRunning >= 0) {
      // kill(alreadyRunning, SIGINT);
      if (addAlarm) {
        //Signal the currently running daemon that the new alarms file was created
        std::cout << "Alerted running instance of new alarm" << std::endl;
        kill(alreadyRunning, SIGUSR1);
      }
      return -3;
    }

    if (daemonized) {
      std::cout << "Cobra reminder daemon started" << std::flush << std::endl;
    } else {
      init_curses();
    }
    init_notify();

    std::thread gthread(gtkLoop,&argc,&argv);

    allAlarms = loadAlarms(ALARM_DIR+ALARM_FILE);
    loadExtraAlarms(allAlarms,ALARM_DIR+ALARM_NEW);
    std::sort(allAlarms.begin(), allAlarms.end(), alarmRemindTimeComparator());

    std::thread othread(outputLoop,daemonized);
    std::thread ithread((daemonized || noinput) ? emptyLoop : inputLoop);

    ithread.join();
    othread.join();
    saveAlarms(allAlarms,ALARM_DIR+ALARM_FILE);
    gtk_main_quit();
    gthread.join();

    if (not daemonized) {
      end_curses();
    }
    end_notify();
    return 0;
  }

  void gtkLoop(int* argc, char*** argv) {
    gtk_init(argc,argv);
    gtk_main();
  }

  void emptyLoop() {

  }

  void inputLoop() {
    int ch;
    highlighted = 0;
    while (not quitOut) {
      if ((ch = getch()) == ERR) {
        usleep(50000);
        continue;
      }
      switch (ch) {
        case KEY_BACKSPACE:
          continue;
        case KEY_UP:
          highlighted -= 1;
          if (highlighted < 0) {
            highlighted = allAlarms.size()-1;
          }
          toUpdate = true;
          continue;
        case KEY_DOWN:
          highlighted += 1;
          if ((unsigned)highlighted == allAlarms.size()) {
            highlighted = 0;
          }
          toUpdate = true;
          continue;
        case 'x':
          if (allAlarms[highlighted].dismissed) {
            allAlarms.erase(allAlarms.begin()+highlighted);
            if (highlighted > 0) {
              highlighted -= 1;
            }
          }
          continue;
        case 'n':
          allAlarms[highlighted].displayed = true;
          allAlarms[highlighted].dismissed = false;
          allAlarms[highlighted].showReminder(highlighted);
          continue;
        case 'q':
          quitOut = true;
          break;
        default:
          continue;
      }
    }
  }

  void outputLoop(bool daemonized) {
    if (daemonized) {
      while (not quitOut) {
        for (unsigned i = 0; i < allAlarms.size(); ++i) {
          if (
            allAlarms[i].displayed ||
            allAlarms[i].dismissed ||
            allAlarms[i].remindTime > time(NULL)
          ) continue;
          allAlarms[i].displayed = true;
          allAlarms[i].showReminder(i);
        }
        usleep(500000);
      }
    } else {
      unsigned updateCounter = 0;
      const char* es = padToLength("displayed",15).c_str();
      const char* ds = padToLength("dismissed",15).c_str();
      while (not quitOut) {
        if (toUpdate || (((++updateCounter) % 10) == 0)) {
          move(0,0);
          wnoutrefresh(stdscr);
          clear();
          // curprint(WHT,0,"%s","Showing all alarms\n");
          for (unsigned i = 0; i < allAlarms.size(); ++i) {
            bool rr = ((unsigned)highlighted == i);
            long ttn = timeToNow(allAlarms[i].remindTime);
            if (ttn > 0) {
              curprint(WHT,rr,"%s",padToLength(msFormat(ttn),15).c_str());
            } else {
              if (allAlarms[i].dismissed || allAlarms[i].displayed) {
                //do nothing
              } else {
                allAlarms[i].displayed = true;
                allAlarms[i].showReminder(i);
              }
              if (allAlarms[i].dismissed) {
                curprint(BLU,rr,"%s",ds);
              } else {
                curprint(GRN,rr,"%s",es);
              }
            }
            curprint(GRN,rr,"%s",allAlarms[i].paddedReminder.c_str());
            curprint(YLW,rr,"%s",allAlarms[i].paddedAction.c_str());
            curprint("\n");
          }
          // wnoutrefresh(stdscr);
          doupdate();
          updateCounter = 0;
          toUpdate = false;
        } else {
          usleep(100000);
        }
      }
    }
  }

  //From http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  Json::Value loadJsonFromFile(std::string fname) {
    std::string jsonstring;
    std::ifstream t(fname);
    t.seekg(0, std::ios::end);
    jsonstring.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    jsonstring.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonstring,root,false);

    return root;
  }

  std::vector<Alarm> loadAlarms(std::string path) {
    std::vector<Alarm> alarms;
    if(file_available(path.c_str())) {
      Json::Value root = loadJsonFromFile(path);
      for (unsigned i = 0; i < root.size(); ++i) {
        // std::cout << files[i] << std::endl;
        alarms.push_back(Alarm(root[i]));
      }
    }

    return alarms;
  }

  void loadExtraAlarms(std::vector<Alarm>& alarms, std::string path) {
    if (not file_available(path.c_str())) {
      return;
    }
    Json::Value root = loadJsonFromFile(path);
    for (unsigned i = 0; i < root.size(); ++i) {
      // std::cout << files[i] << std::endl;
      alarms.push_back(Alarm(root[i]));
    }
    saveAlarms(alarms,ALARM_DIR+ALARM_FILE);
    std::remove(path.c_str());
  }

  void saveAlarms(std::vector<Alarm> alarms, std::string fname) {
    Json::Value root;
    for (unsigned i = 0; i < alarms.size(); ++i) {
      alarms[i].updateData();
      root[i] = alarms[i].j;
    }

    std::ofstream ofile;
    ofile.open(fname);
    Json::StyledWriter styledWriter;
    ofile << styledWriter.write(root);
    ofile.close();
  }

}
