#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <list>
#include <cstring>
#include <unistd.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
//dksjk
class Command {
// TODO: Add your data members
    const char* cmd_line;
    int args_num;
    char** cmd_parsed; //array of strings. each item in the array is a word

 public:
  Command(const char* new_cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
  const char* getCmdLine();
  int getArgsNum();
  char** getCmdParsed();
  void removeAmpersand(); //remove ampersand from cmd_parsed;
  void changeCmdParsed(const char* new_cmd_line); // changes the spaces and args num
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChpromptCommand : public BuiltInCommand {

public:
    ChpromptCommand(const char* cmd_line);
    virtual ~ChpromptCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  char** p_last_pwd;
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;                                           //Scanned
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
  QuitCommand(const char* cmd_line);
  virtual ~QuitCommand() {}
  void execute() override;
};


class JobsList {
 public:
  class JobEntry {
      int job_id;
      pid_t job_pid;
      bool is_stopped;
      time_t begin_time;
      const char* command_line;
  public:
      JobEntry(int job_id, pid_t job_pid, bool is_stopped, time_t begin, const char* new_command_line);
      ~JobEntry();
      int getJobId();
      pid_t getJobPID();
      bool isStopped();
      time_t getBeginTime();
      void stop(); //stops a job (not stopped anymore)
      void cont(); //continues a job (not stopped anymore)
      const char* getCommandLine();
  };
 // TODO: Add your data members
 std::list<JobEntry*> jobs_list;
 int max_job_id=0;
 public:
  JobsList() =default;
  ~JobsList() = default;
  void addJob(const char* cmd_line, pid_t new_pid, bool isStopped = false); //DONE - changed Command to char*
  void printJobsList(); //DONE
  void killAllJobs();                 //DONE
  void removeFinishedJobs();          //DONE
  JobEntry * getJobById(int jobId);   //DONE
  int size();//nezo 3ezr

  JobEntry * getJobByPid(int jobId);
  //void removeJobByPid(int jobPid);      //DONE
  void removeJobByPid(int jobPid);      //DONE
  JobEntry * getLastJob(int* lastJobId); //DONE
  JobEntry *getLastStoppedJob(int *jobId); //DONE
  // TODO: Add extra methods or modify exisitng ones as needed
  void updateMaxId(); //DONE
  void printJobsListWithoutSpes(); //doesn't print if stopped or not
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Optional */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class FareCommand : public BuiltInCommand {
  /* Optional */
  // TODO: Add your data members
 public:
  FareCommand(const char* cmd_line);
  virtual ~FareCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  /* Optional */
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
  /* Bonus */
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell {
private:
    char *output_line;
    //char *last_pwd;
    JobsList* jobs_list;
    char* curr_cmd;
    pid_t curr_pid;
    bool isFG;

    // TODO: Add your data members
    SmallShell() //because of singleton design, user access to constructor is prevented.
    // Users can only use getInstance method
    {
        output_line = new char[80];
        strcpy(output_line, "smash> ");
        last_pwd = new char[strlen("") + 1];
        strcpy(last_pwd, "");
        jobs_list = new JobsList();
        curr_cmd = new char[COMMAND_MAX_ARGS];
        isFG = true;
        curr_pid = getpid();
        if(curr_pid == -1)
        {
            perror("smash error: getpid failed");
        }
    }

 public:
    char *last_pwd;
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
  char* getOutputLine();
  void setOutputLine(char* new_output_line);
  char* getLastPWD();
  JobsList* getJobsList();
  int getJobsListSize();//nezo 3ezr


  void setCurrPid(pid_t new_pid);
  void setBG(); //changes isFG to false
  pid_t getCurrPid();
  bool isFGProcess();
  char* getCurrCmd();
};

#endif //SMASH_COMMAND_H_
