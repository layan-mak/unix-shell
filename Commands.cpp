#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>


using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";
const std::string REDIRECT_SYMB = ">";
const std::string PIPE_SYMB = "|";
const std::string AND_SYMB = "&";


#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

/** Helper Functions: */

/**
 * to_convert: a char* containing a number
 * return value: the number that was in to_convert represented by int
 * */
int convert_char_string_to_int(char* to_convert)
{
    std::string string_id = to_convert;
    return stoi(string_id);
}

/**
 * to_check: a string that we want to check if it contains only numnbers(digits)
 * */
bool all_are_digits(char* to_check)
{
    bool all_are_digits = true;
    const char* tmp_char = to_check;
    if(tmp_char[0] == '-')
    {
		tmp_char++;
	}
    for (int i = 0; i < (int)(strlen(tmp_char)); i++)
    {
        if(!isdigit(tmp_char[i]))
        {
            all_are_digits = false;
            break;
        }
    }
    return all_are_digits;
}

bool isExternalCommand(const char* cmd_line)
{
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("pwd") == 0 || firstWord.compare("showpid") == 0 || firstWord.compare("chprompt") == 0
    || firstWord.compare("cd") == 0 || firstWord.compare("jobs") == 0 ||  firstWord.compare("quit") == 0)
    {
        return false;
    }
    auto ptr1 = strchr(cmd_line, '>');
    auto ptr2 = strchr(cmd_line, '|');

    if(ptr1 != nullptr || ptr2 != nullptr) //redirection/pipe command
    {
        return false;
    }
    return true;
}

/**
 * this function accepts a command line which contains a redirection symbol and
 * returns the line before this symbol
 * */
//IMPORTANT: Memory has been allocated for return value! Need to delete after use.
const char* before_symb(const char* cmd_line, string symb)
{
    string tmp_cmd = cmd_line;
    int first_occur = tmp_cmd.find_first_of(symb); //starts counting from 0
    string before = tmp_cmd.substr(0, first_occur);
    int size = before.size();
    char* char_before = new char[size + 1];
    strcpy(char_before, before.c_str());
    return char_before;
}

//IMPORTANT: Memory has been allocated for return value! Need to delete after use.
const char* after_symb(const char* cmd_line, bool and_symb)
{
    string tmp_cmd = cmd_line;
    int first_occur = tmp_cmd.find_first_of(PIPE_SYMB); //starts counting from 0
    string before = tmp_cmd.substr(0, first_occur);
    string s_after_symb;
    if(!and_symb) // symb = '|'
    {
        s_after_symb = tmp_cmd.substr(first_occur + 1,  strlen(tmp_cmd.c_str()) + 1);
    }
    else // symb = '|&'
    {
        s_after_symb = tmp_cmd.substr(first_occur + 2,  strlen(tmp_cmd.c_str()) + 1);
    }
    char* char_after_symb = new char[s_after_symb.size() + 1];
    strcpy(char_after_symb, s_after_symb.c_str());
    return char_after_symb;
}

/**this function gets the cmd_line and adds spaces before and after every redirection symbol
assumes that redirection symbols are not in the beginning or last of
 */
//IMPORTANT: Memory has been allocated for return value! Need to delete after use.
const char* change_cmd_line_redirect(const char* cmd_line)
{
    string tmp_cmd = cmd_line;
    int first_occur = tmp_cmd.find_first_of(REDIRECT_SYMB); //starts counting from 0
    string before_symb = tmp_cmd.substr(0, first_occur);
    bool one_symb = true; //assume that the symbol is ">" and not ">>"
    if(tmp_cmd[first_occur + 1] == '>')
    {
        one_symb = false;
    }
    string after_symb;
    string converted_cmd;
    if(one_symb)
    {
        after_symb = tmp_cmd.substr(first_occur + 1, strlen(tmp_cmd.c_str()) + 1);
        converted_cmd = before_symb + " > " + after_symb;
    }
    else
    {
        after_symb = tmp_cmd.substr(first_occur + 2, strlen(tmp_cmd.c_str()) + 1);
        converted_cmd = before_symb + " >> " + after_symb;
    }
    char* converted_char = new char[converted_cmd.size() + 1];
    strcpy(converted_char, converted_cmd.c_str());
    return converted_char;
}


const char* change_cmd_line_pipe(const char* cmd_line)
{
    string tmp_cmd = cmd_line;
    int first_occur = tmp_cmd.find_first_of(PIPE_SYMB); //starts counting from 0
    string before_symb = tmp_cmd.substr(0, first_occur);
    bool one_symb = true; //assume that the symbol is "|" and not "|&"
    if(tmp_cmd[first_occur + 1] == '&')
    {
        one_symb = false;
    }
    string after_symb;
    string converted_cmd;
    if(one_symb)
    {
        after_symb = tmp_cmd.substr(first_occur + 1, strlen(tmp_cmd.c_str()) + 1);
        converted_cmd = before_symb + " | " + after_symb;
    }
    else
    {
        after_symb = tmp_cmd.substr(first_occur + 2, strlen(tmp_cmd.c_str()) + 1);
        converted_cmd = before_symb + " |& " + after_symb;
    }
    char* converted_char = new char[converted_cmd.size() + 1];
    strcpy(converted_char, converted_cmd.c_str());
    return converted_char;
}

/*return the index of the char* in cmd_parsed which contains the redirection symbol
changed it!! important to notice removal of line with i++ ! */
int find_symb_arg_in_cmd_parsed(char** parsed, int args_num, char symb)
{
    int i = 0;
    for(i = 0; i < args_num; i++)
    {
//        char ch = parsed[i][0];
        if(parsed[i][0] == symb)
        {
            return i;
        }
    }
    return -1;
}

/** Command Class Implementation: */
Command::Command(const char *new_cmd_line) : cmd_parsed(new char*[COMMAND_MAX_ARGS])
{
    char* tmp = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(tmp, new_cmd_line);
    cmd_line = tmp;
    args_num = _parseCommandLine(cmd_line, cmd_parsed);
}

Command::~Command()
{
    delete cmd_line;
    delete cmd_parsed;
}

const char* Command::getCmdLine()
{
    return cmd_line;
}

int Command::getArgsNum()
{
    return args_num;
}

char** Command::getCmdParsed()
{
    return cmd_parsed;
}

void Command::removeAmpersand()
{
    char* temp_cmd_line = strdup(cmd_line);
    _removeBackgroundSign(temp_cmd_line);
    _parseCommandLine(temp_cmd_line, cmd_parsed);
}

void Command::changeCmdParsed(const char* new_cmd_line)
{
    delete cmd_parsed;
    cmd_parsed = new char*[COMMAND_MAX_ARGS];
    int new_args_num = _parseCommandLine(new_cmd_line, cmd_parsed);
    args_num = new_args_num;
}


/**SmallShell Class Implementation: */
SmallShell::~SmallShell()
{
    delete[] output_line;
    delete[] last_pwd;
    delete jobs_list;
    delete[] curr_cmd;
}

char* SmallShell::getLastPWD()
{
    return last_pwd;
}

char *SmallShell::getOutputLine()
{
    return output_line;
}

void SmallShell::setOutputLine(char *new_output_line)
{
    strcpy(output_line, new_output_line);
}

void SmallShell::setCurrPid(pid_t new_pid)
{
    curr_pid = new_pid;
}

void SmallShell::setBG()
{
    isFG = false;
}

pid_t SmallShell::getCurrPid()
{
    return curr_pid;
}
bool SmallShell::isFGProcess()
{
    return isFG;
}
char* SmallShell::getCurrCmd()
{
    return curr_cmd;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    //add pipe  DONE
    //in validargs: removeampersand() also in redirection DONE
    //update isExternal function to test pipe+redirection DONE
    auto ptr1 = strchr(cmd_line, '>');
    auto ptr2 = strchr(cmd_line, '|');

    if(ptr1 != nullptr) //redirection command
    {
        return new RedirectionCommand(cmd_line);
    }
    else if(ptr2 != nullptr) //pipe command
    {
        return new PipeCommand(cmd_line);
    }
    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("chprompt") == 0)
    {
        return new ChpromptCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0)
    {
        return new ChangeDirCommand(cmd_line, &(this->last_pwd));
    }
    else if (firstWord.compare("jobs") == 0)
    {
        return new JobsCommand(cmd_line);
    }
    else if (firstWord.compare("fg") == 0)
    {
        return new ForegroundCommand(cmd_line);
    }
    else if (firstWord.compare("bg") == 0)
    {
        return new BackgroundCommand(cmd_line);
    }
    else if (firstWord.compare("quit") == 0)
    {
        return new QuitCommand(cmd_line);
    }
    else if(firstWord.compare("kill") == 0)
    {
        return new KillCommand(cmd_line);
    }
     else
     {
       return new ExternalCommand(cmd_line);
     }

    return nullptr;
}

//check for signals? maybe move the if statements to execute of external commands?
void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    Command* cmd = CreateCommand(cmd_line);
    jobs_list->removeFinishedJobs();
    if(!isExternalCommand(cmd_line))
    {
        isFG = false;
    }
    else if(isExternalCommand(cmd_line) && _isBackgroundCommand(cmd_line))
    {

        isFG = false;
    }
    else if(isExternalCommand(cmd_line) && !_isBackgroundCommand(cmd_line))
    {
        isFG = true;
    }

    curr_pid = getpid();
    if(curr_pid == -1)
    {
        perror("smash error: getpid failed");
        return;
    }
    strcpy(curr_cmd, cmd_line);
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

JobsList* SmallShell::getJobsList()
{

    return jobs_list;
}


//nezo 3ezr
int SmallShell::getJobsListSize()
{

    return jobs_list->size();
}

//BuiltInCommand Class Implementation

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {} ;


//ExternalCommand Class Implementation:

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {} ;

void ExternalCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();

	pid_t smash_pid = getpid();
	if(smash_pid == -1)
	{
		perror("smash error: getpid failed");
        return;
    }
    
    pid_t fork_res = fork();
    if(fork_res < 0)             // fork failed
    {
        perror("smash error: fork failed");
        return;
    }
    if(fork_res > 0)       //parent process
    {
		smash.setCurrPid(fork_res);
		//layan added
		//cout << "external command pid is: " << fork_res <<endl;
        if(_isBackgroundCommand(getCmdLine()))
        {

            //fork_res contains child pid
            smash.getJobsList()->addJob(getCmdLine(), fork_res, false);

        }
        else
        {
            int wait_res=waitpid(fork_res, nullptr,WUNTRACED);
            if(wait_res== -1)
            {
                perror("smash error: waitpid failed");
                return;
            }
            //added this! two lines layan
            smash.setCurrPid(smash_pid);
            smash.setBG();
        }
//        return;
    }
    else//(fork_res == 0)      //child process
    {

        setpgrp();
        
        pid_t new_pid = getpid();
        if(new_pid == -1)
        {
            perror("smash error: getpid failed");
            return;
        }
        
        smash.setCurrPid(new_pid);
        
        if(strchr(getCmdLine(), '*') || strchr(getCmdLine(), '?')) //complex command
        {
            removeAmpersand();

            //need to new?
            char* argv[] = {(char*) "/bin/bash", (char*) "-c",
                            (char*) getCmdParsed(), (char*) nullptr};

            int res = execv(argv[0], argv);
            if(res == -1)
            {
                perror("smash error: execv failed");
                exit(0);
            }
            return;
        }
        else //executable command
        {

            removeAmpersand();

            int res = execvp(getCmdParsed()[0], getCmdParsed());
            if(res == -1)
            {
                perror("smash error: execvp failed");
                exit(0);

            }
        }
        //smash.setCurrPid(smash_pid);
        //exit(0);
//        return;
    }

}

//JobEntry and JobsList Class Implementation

JobsList::JobEntry::JobEntry(int job_id, pid_t job_pid, bool is_stopped, time_t begin,
                             const char* new_command_line) : job_id(job_id), job_pid(job_pid),
                             is_stopped(is_stopped), begin_time(begin) {
    char* tmp = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(tmp, new_command_line);
    command_line = tmp;
}

JobsList::JobEntry::~JobEntry()
{
    delete command_line;
}

int JobsList::JobEntry::getJobId()
{
    return job_id;
}

pid_t JobsList::JobEntry::getJobPID()
{
    return job_pid;
}

bool JobsList::JobEntry::isStopped()
{
    return is_stopped;
}

time_t JobsList::JobEntry::getBeginTime()
{
    return begin_time;
}

const char* JobsList::JobEntry::getCommandLine()
{
    return command_line;
}

void JobsList::JobEntry::cont()
{
    is_stopped = false;
}

void JobsList::JobEntry::stop() //stops a job (not stopped anymore)
{
    is_stopped = true;
}

void JobsList::addJob(const char* cmd_line, pid_t new_pid, bool isStopped)
{
    removeFinishedJobs();
    time_t new_time = time(nullptr);
    if(new_time == (time_t) -1)
    {
        perror("smash error: time failed");
        return;
    }

    JobEntry* new_job = new JobEntry(max_job_id + 1, new_pid ,isStopped,
                                     new_time, cmd_line);
    jobs_list.insert(jobs_list.end(), new_job);

    updateMaxId();
}

void JobsList::printJobsListWithoutSpes()
{
    removeFinishedJobs();
    std::cout<< "smash: sending SIGKILL signal to " << jobs_list.size() << " jobs:" << std::endl;
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        std::cout << (*iterator)->getJobPID() << ": " << (*iterator)->getCommandLine() << std::endl;
        ++iterator;
    }
    return;
}

void JobsList::printJobsList()
{

    removeFinishedJobs();
    updateMaxId();
    auto iterator = jobs_list.begin();

    while(iterator != jobs_list.end())
    {
        time_t curr_time = time(nullptr);
        if(curr_time == (time_t) -1)
        {
            perror("smash error: time failed");
            return;
        }
        time_t diff = difftime(curr_time, (*iterator)->getBeginTime());
//        if(diff == (time_t) -1)
//        {
//            perror("smash error: difftime failed");
//            return;
//        }

        if((*iterator)->isStopped())
        {
            std::cout << "[" << (*iterator)->getJobId() << "] " << (*iterator)->getCommandLine()
                                            << " : " << (*iterator)->getJobPID()
                                            << " " << diff << " secs"
                                            <<" (stopped)" << std::endl;
        }
        else
        {
            std::cout << "[" << (*iterator)->getJobId() << "] " << (*iterator)->getCommandLine()
                        << " : " <<
                      (*iterator)->getJobPID() << " " << diff << " secs" << std::endl;
        }
        ++iterator;
    }
}

void JobsList::killAllJobs()
{
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        int kill_result = kill((*iterator)->getJobPID(), SIGKILL);
        if(kill_result == -1)
        {
            perror("smash error: kill failed");
            return;
        }
        delete (*iterator);
        ++iterator;
    }
    jobs_list.clear();
} //signals?

//int count=1;


void JobsList::removeFinishedJobs()
{
//    std::cout<<"count = "<<count<<std::endl;
//    count++;

    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {

        int status = 0;
        pid_t wait_res = (waitpid( (*iterator)->getJobPID(), &status , WNOHANG));
        //cout << "process pid = " <<(*iterator)->getJobPID() << endl;
        //if(wait_res == -1)
        //{
        //  cout << "I'm in removefinishedjobs" << endl;
        //    perror("smash error: waitpid failed");
        //    return;
        //}
        if(wait_res != 0) //wait_res ==0 if the process's state hasn't changed(hasn't finished)
        //if(WIFEXITED(status) || wait_res != 0) //if job is finished
        {

            //delete job
            delete (*iterator);
            jobs_list.erase(iterator);
            iterator = jobs_list.begin();
        }
        ++iterator;
    }
    updateMaxId();


}

JobsList::JobEntry* JobsList::getJobById(int jobId)
{
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        if (jobId==((*iterator)->getJobId()))
        {
            return (*iterator);
        }
        ++iterator;
    }
    return nullptr;
}


JobsList::JobEntry* JobsList::getJobByPid(int jobPid)
{
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        if (jobPid==((*iterator)->getJobPID()))
        {

            return (*iterator);
        }
        ++iterator;
    }
    return nullptr;
}


int JobsList::size()
{
    return jobs_list.size();
}




void JobsList::removeJobByPid(int jobPid)
{
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        if (jobPid == (*iterator)->getJobPID())
        {
            delete (*iterator);
            jobs_list.erase(iterator);
            iterator = jobs_list.end(); //there is only one job id
        }
        ++iterator;
    }
    updateMaxId();
    return;
}

//see fg command
//this function gets the job with the maximal job id (don't care if background process or stopped)
JobsList::JobEntry * JobsList::getLastJob(int* lastJobId)
{
    /*int last_job=0;
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        if (last_job < (*iterator)->getJobId())
        {
            last_job=(*iterator)->getJobId();
        }
        ++iterator;
    }
*/
    *lastJobId = max_job_id;
    return getJobById(max_job_id);
}

//see bg command
JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId)
{
    int last_job = 0;
    *jobId = 0;
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        if (last_job < (*iterator)->getJobId())
        {
            if((*iterator)->isStopped())
            {
                last_job=(*iterator)->getJobId();
            }
        }
        ++iterator;
    }
    *jobId = last_job;
    return getJobById(last_job);
}




void JobsList::updateMaxId()
{
    int max_id=0;
    auto iterator = jobs_list.begin();
    while(iterator != jobs_list.end())
    {
        if ((*iterator) && (max_id < (*iterator)->getJobId()))
        {
            max_id=(*iterator)->getJobId();
        }
        ++iterator;
    }
    max_job_id = max_id;
}




/* ------------------------------- SPECIAL COMMANDS --------------------------*/

/**
 * PipeCommand Class Implementation:
 * */
PipeCommand::PipeCommand(const char *cmd_line) :Command(cmd_line){};

//returns false if the first or last charachter in cmd_line is pipe symbol
bool validArgumentsPipe(char** parsed, int num_args)
{
    int last_char_index = strlen(parsed[num_args -1]) - 1;
    if(parsed[0][0] == '|' || parsed[num_args-1][last_char_index] == '|'
    || parsed[num_args - 1][last_char_index] == '&')
    {
        return false;
    }
    return true;
}

//returns true if the symbol is '|&' and false if the symbol is '|'
bool and_symbol(char* symb)
{
    string single_symb = "|";
    string double_symb = "|&";
    if(strcmp(symb, single_symb.c_str()) == 0)
    {
        return false;
    }
    return true;
}
void PipeCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    char** parsed = getCmdParsed();
    //remove ampersand
    if(!validArgumentsPipe(parsed, getArgsNum()))
    {
        std::cerr << "smash error:> " << getCmdLine() << std::endl;
        return;
    }

    const char* changed_cmd = change_cmd_line_pipe(getCmdLine());
    changeCmdParsed(changed_cmd);
    parsed = getCmdParsed();
    const char* cmd_line_before_symb = before_symb(getCmdLine(), PIPE_SYMB);
    int symb_index_in_parsed = find_symb_arg_in_cmd_parsed(parsed, getArgsNum(), '|');
    bool and_symb_exists = and_symbol(parsed[symb_index_in_parsed]);
    const char* cmd_line_after_symb = after_symb(getCmdLine(), and_symb_exists);
    int pipe_arr[2];   //need to new?
    int pipe_res = pipe(pipe_arr);
    if(pipe_res == -1)
    {
        perror("smash error: pipe failed");
        return;
    }
    int fork_res = fork();      //fork1
    if(fork_res == -1)
    {
        perror("smash error: fork failed");
        return;
    }
    if(fork_res == 0)
    {
        pid_t set_res = setpgrp();
        if(set_res == -1)
        {
            perror("smash error: setpgrp failed");
            return;
        }
        int dup_res;
        if(and_symb_exists) //stderr
        {
            dup_res = dup2(pipe_arr[1], 2);
        }
        else
        {
            dup_res = dup2(pipe_arr[1], 1);
        }
        if(dup_res == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }

        if(close(pipe_arr[0]) == -1 || close(pipe_arr[1]) == -1)
        {
            perror("smash error: pipe failed");
            return;
        }
        smash.executeCommand(cmd_line_before_symb);
        exit(0);
    }

    pid_t fork_res2 = fork();          //fork2
    if(fork_res2 == -1)
    {
        perror("smash error: fork failed");
        return;
    }

    if(fork_res2 == 0)
    {
        int set_res2 = setpgrp();
        if(set_res2 == -1)
        {
            perror("smash error: setpgrp failed");
            return;
        }
        int dup_res = dup2(pipe_arr[0], 0);//nezo hon - layan: manghshek!! btktbi zyi!❤️
        if(dup_res == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }
        if(close(pipe_arr[0]) == -1 || close(pipe_arr[1]) == -1){
            perror("smash error: pipe failed");
            return;
        }
        smash.executeCommand(cmd_line_after_symb);
        exit(0);

    }

    if(close(pipe_arr[0]) == -1 || close(pipe_arr[1]) == -1){
        perror("smash error: pipe failed");
        return;
    }


    int waitpid_res=waitpid(fork_res, nullptr, 0);
    if(waitpid_res == -1)
    {

        perror("smash error: waitpid failed");
        return;
    }
    waitpid_res=waitpid(fork_res2, nullptr, 0);
    if(waitpid_res == -1)
    {

        perror("smash error: waitpid failed");
        return;
    }
    delete[] changed_cmd;
    delete[] cmd_line_before_symb;
    delete[] cmd_line_after_symb;
    return;
}

//removed printing from this function.
bool validArgumentsRedirection(char** parsed, int args_num)
{
    int last_word_index = args_num -1;
    int last_char_in_string_index = strlen(parsed[args_num-1]) - 1;
    if(parsed[0][0] == '>' || parsed[last_word_index][last_char_in_string_index]=='>')
    {
        //std::cerr << "smash error:> " << getCmdLine() << std::endl;
        return false;
    }
    return true;
}

//returns true if the symbol is '>>' and false if the symbol is '>'
bool append_symbol(char* symb)
{
    string single_symb = ">";
    string double_symb = ">>";
    if(strcmp(symb, single_symb.c_str()) == 0)
    {
        return false;
    }
    return true;
}


//RedirectionCommand Class Implementation
RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line) {} ;

void RedirectionCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    char** parsed = getCmdParsed();
    if(!validArgumentsRedirection(parsed, getArgsNum()))
    {
        std::cerr << "smash error:> " << getCmdLine() << std::endl;
        return;
    }
   
    const char* changed_cmd = change_cmd_line_redirect(getCmdLine());
  
    changeCmdParsed(changed_cmd);
    parsed = getCmdParsed(); //check for bugs.. checked - all good!
    // change cout fdt:
    
    int fdt_index = dup(1);
    if(fdt_index == -1)
    {
        perror("smash error: dup failed");
        return;
    }
    
    
    int index = find_symb_arg_in_cmd_parsed(parsed, getArgsNum(), '>');
    string path = parsed[index + 1];
	int open_res;
    if(append_symbol(parsed[index]))
    {
        open_res = open(path.c_str(), O_RDWR | O_APPEND | O_CREAT, 0655);
        if(open_res == -1)
        {
            perror("smash error: open failed");
            return;
        }
    }
    else
    {
        open_res = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0655);
        if(open_res == -1)
        {
            perror("smash error: open failed");
            return;
        }
    }
    
    int close_res = close(1);
    if(close_res == -1)
    {
        perror("smash error: close failed");
        return;
    }

	int dup2_res = dup2(open_res, 1);
	if(dup2_res == -1)
	{
		perror("smash error: dup2 failed");
        return;
	}
    const char* cmd_line_without_symb = before_symb(getCmdLine(), REDIRECT_SYMB);
    smash.executeCommand(cmd_line_without_symb);
    delete[] cmd_line_without_symb;
    
    close_res = close(open_res);
    if(close_res == -1)
    {
        perror("smash error: close failed");
        return;
    }
    close_res = close(1);
    if(close_res == -1)
    {
        perror("smash error: close failed");
        return;
    }
    dup2_res = dup2(fdt_index, 1);
    if(dup2_res == -1)
	{
		perror("smash error: dup2 failed");
        return;
	}
    delete[] changed_cmd;
    return;
}

//FareCommand Class Implementation
FareCommand::FareCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {} ;

void FareCommand::execute()
{
    //SmallShell& smash = SmallShell::getInstance();
    if(getArgsNum() != 4)
    {
        cerr << "smash error: fare: invalid arguments" << endl;
        return;
    }
    char** parsed = getCmdParsed();
    FILE* source_file = fopen(parsed[1],"r+");
    FILE* temp_file = tmpfile();
    int count = 0;
    int source_len = strlen(parsed[2]);
    int file_pointer = 0;
    char curr[source_len + 1];

    while(fgets(curr,source_len + 1,source_file) != nullptr)
    {
        if(strcmp(parsed[2],curr) == 0)
        {
            count++;
            file_pointer += source_len;
            fputs(parsed[3],temp_file);
        } else{
            file_pointer++;
            fputc(curr[0],temp_file);
        }
        fseek(source_file,file_pointer,SEEK_SET);
    }

    fseek(temp_file,0,SEEK_SET);

    ///to delete all the content of the file
    fclose(source_file);
    source_file = fopen(parsed[1],"w+");

    ///copy the requested content of the file
    char restore = fgetc(temp_file);
    while (restore != EOF)
    {
        fputc(restore, source_file);
        restore = fgetc(temp_file);
    }
    fclose(temp_file);
    fclose(source_file);

	std::cout << "replaced " << count << " instances of the string " << "\"" << parsed[2] <<
    "\"" << endl;
	
    return;
}

/* ------------------------------- BUILT IN COMMANDS -------------------------*/

//Chprompt Class Implementation:
ChpromptCommand::ChpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {} ;

void ChpromptCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    if(getArgsNum() == 1)
    {
        string new_ol = "smash> ";
        smash.setOutputLine((char*)(new_ol.c_str()));
    }
    else //(args_num >= 2)
    {
        char* new_line = strcat(getCmdParsed()[1], "> ");
        smash.setOutputLine(new_line);
    }
    return;
}

//ShowPID Class Implementation:
ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {} ;

void ShowPidCommand::execute()
{
    //SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    pid_t smash_pid = getpid();
    std::cout << "smash pid is " << smash_pid << std::endl;
    return;
}

//GetCurrDirCommand (PWD) Class Implementation:
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {} ;

void GetCurrDirCommand::execute()
{
    //SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    char* path = getcwd(NULL, 0);
    if(path == nullptr)
    {
        perror("smash error: getcwd failed");
        return;
    }
    else
    {
        std::cout << path << std::endl;
    }
    return;
}

//CD Class Implementation:
ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand(cmd_line),
                                                                            p_last_pwd(plastPwd) {} ;
void ChangeDirCommand::execute()
{
    char** cmd_parsed = getCmdParsed();
    //SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    if(getArgsNum() > 2)
    {
        std::cerr << "smash error: cd: too many arguments" << endl;
        return;
    }
    if(strcmp(cmd_parsed[1],"-")==0) //go to last pwd
    {
        if(strcmp(*p_last_pwd,"")==0) //if last pwd==is empty
        {
            std::cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        else
        {
            //update last_pwd and update the current wd
            char* curr_pwd = getcwd(NULL, 0);
            int res = chdir(*p_last_pwd);
            if(res == -1)
            {
                perror("smash error: chdir failed");
                return;
            }
            else
            {
                delete (*p_last_pwd);
                *p_last_pwd = new char[strlen(curr_pwd) + 1];
                strcpy(*p_last_pwd, curr_pwd);
            }
        }
    }
    else
    {
        char* curr_pwd = getcwd(NULL, 0);
        int res = chdir(cmd_parsed[1]);
        if(res == -1)
        {
            perror("smash error: chdir failed");
        }
        else
        {
            delete (*p_last_pwd);
            *p_last_pwd = new char[strlen(curr_pwd) + 1];
            strcpy(*p_last_pwd, curr_pwd);
        }
    }
    return;
}

//JobsCommand Class Implementation:
JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line){} ;

void JobsCommand::execute()
{

    SmallShell& smash = SmallShell::getInstance();


//    if(smash.getJobsList()->size()==0)
//    {
//        std::cout<<"the jobs list is empty"<<std::endl;
//    }
    removeAmpersand();

    smash.getJobsList()->printJobsList();
    return;
}


//QuitCommand Class Implementation:
QuitCommand::QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line){} ;

void QuitCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    if(getArgsNum() == 1)
    {
        smash.getJobsList()->killAllJobs();
        exit(0);
    }
    if(getArgsNum() == 2)
    {
        char* second_arg = getCmdParsed()[1];
        if(strcmp(second_arg, "kill") != 0)
        {
            std::cerr << "smash error: quit: invalid arguments" << std::endl;
            return;
        }
        smash.getJobsList()->printJobsListWithoutSpes();
        smash.getJobsList()->killAllJobs();
    }
    exit(0);
}



//ForegroundCommand FG Class Implementation:
ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {} ;

void ForegroundCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    if(getArgsNum() > 2)
    {
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }
    pid_t job_pid;
    int job_id;
    JobsList::JobEntry* job_entry;
    if(getArgsNum() == 1) //no job id was specified
    {
        job_entry = smash.getJobsList()->getLastJob(&job_id);
//        std::cout<<"the job id is : "<<job_id<<std::endl;
        if(job_entry == nullptr)
        {
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
            return;
        }
        job_pid = job_entry->getJobPID();
    }
    else if(getArgsNum() == 2)
    {
        //check valid arguments
        if(!all_are_digits(getCmdParsed()[1]))
        {

            std::cerr << "smash error: fg: invalid arguments" << std::endl;
            return;
        }
        char* char_id = getCmdParsed()[1];
        //convert job_id from char* to int
        job_id = convert_char_string_to_int(char_id);

        job_entry = smash.getJobsList()->getJobById(job_id);
        if(job_entry == nullptr) //check if this job_id exists
        {
            std::cerr << "smash error: fg: job-id " << job_id << " does not exist" << std::endl;
            return;
        }
        job_pid = job_entry->getJobPID();
    }
        std::cout << job_entry->getCommandLine() << " : " << job_pid << std::endl;
        smash.setCurrPid(job_pid);
     
        int kill_res = kill(job_pid, SIGCONT);
        
        if(kill_res == -1)
        {
            perror("smash error: kill failed");
            return;
        }
       
        int waitpid_res = waitpid(job_pid, nullptr, WUNTRACED);
       
        if(waitpid_res == -1)
        {

            perror("smash error: waitpid failed");
            return;
        }
		
        return;
}


//BackgroundCommand BG Class Implementation:
BackgroundCommand::BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {} ;

void BackgroundCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    removeAmpersand();
    if(getArgsNum() > 2)
    {
        std::cerr << "smash error: bg: invalid arguments" << std::endl;
        return;
    }
    JobsList::JobEntry* job_entry;
    int job_id;
    pid_t job_pid;
    if(getArgsNum() == 1)
    {
        job_entry = smash.getJobsList()->getLastStoppedJob(&job_id);
        if(job_entry == nullptr) //no stopped processes in the list
        {
            std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
            return;
        }
        job_pid = job_entry->getJobPID();
    }
    else if(getArgsNum() == 2)
    {
        if(!all_are_digits(getCmdParsed()[1]))
        {
            std::cerr << "smash error: bg: invalid arguments" << std::endl;
            return;
        }
        job_id = convert_char_string_to_int(getCmdParsed()[1]);
        job_entry = smash.getJobsList()->getJobById(job_id);
        if(job_entry == nullptr)
        {
            std::cerr << "smash error: bg: job-id " << job_id << " does not exist" << std::endl;
            return;
        }
        if(!job_entry->isStopped())
        {
            std::cerr << "smash error: bg: job-id " << job_id << " is already running in the background"
                            << std::endl;
            return;
        }
        job_pid = job_entry->getJobPID();
    }
    std::cout << job_entry->getCommandLine() << " : " << job_pid << std::endl;
    int kill_res = kill(job_pid, SIGCONT);
    if(kill_res == -1)
    {
        perror("smash error: kill failed");
        return;
    }
    //job_entry = smash.getJobsList()->getJobById(job_id);
    job_entry->cont();
    return;
}

//KillCommand Class Implementation
KillCommand::KillCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {} ;

void KillCommand::execute()
{
	//cerr << "just entered kill command!" <<endl;
    SmallShell& smash = SmallShell::getInstance();
    char** parsed = getCmdParsed();
    if(getArgsNum() != 3)
    {
        //layan hon
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    else if (!all_are_digits(parsed[1]) || !all_are_digits(parsed[2])) //just added
    {
		cerr << "smash error: kill: invalid arguments" << endl;
        return;
	}
    int sig_num = atoi(parsed[1]);
    int job_id = atoi(parsed[2]);
    sig_num = -1 * sig_num;
    if(sig_num <= 0 || sig_num > 31 || job_id==0)
    {
		//cerr << "the job id is: " << job_id << endl;
		//cerr << "the signal number is: " << sig_num << endl;
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    JobsList::JobEntry* job = smash.getJobsList()->getJobById(job_id);
    if(job == nullptr)
    {
        cerr << "smash error: kill: job-id " << job_id << " does not exist"<< endl;
        return;
    }
    pid_t job_pid = job->getJobPID();
    int kill_res = kill(job_pid, sig_num);
    if(kill_res == -1)
    {
		cerr << "the job id is: " << job_id << endl;
		cerr << "the signal number is: " << sig_num << endl;
        perror("smash error: kill failed");
        return;
    }
    cout << "signal number " << sig_num << " was sent to pid " << job_pid << endl;
    if(sig_num == 9) //this was added!
    {
		smash.getJobsList()->removeJobByPid(job_pid);
	}
	//cerr<<"just exited kill command!"<<endl;
}

/* ---------------------------------------------------------------------------*/

