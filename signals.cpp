#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include "unistd.h"
#include <sys/wait.h>

using namespace std;

void ctrlCHandler(int sig_num)
{
    SmallShell& smash = SmallShell::getInstance();
    cout << "smash: got ctrl-C" << endl;
    pid_t curr_pid = smash.getCurrPid();
    pid_t smash_pid = getpid();
    
    
    //cout<< "curr pid is: "<< curr_pid <<endl;
    //cout<< "smash pid is: "<< smash_pid <<endl;
    if(smash_pid == -1)
    {
		perror("smash error: getpid failed");
        return;
	}
	if(!smash.isFGProcess())
    {
		//cout<<"not fg"<<endl;
        return;
    }
	if(curr_pid == smash_pid)
	{
		//cout<<"same"<<endl;
		//cout<<"smash>"<<endl;
		return;
	}
    //if there's a process running in the foreground:
	
	//check if the process has finished before sending sigkill.. (check if pid exists!)
	//if(kill(curr_pid, 0) != 0 ) //kill(pid,0) == 0 means the pid exists.
	//{
	//	return;
	//}
	
	//int status = 0;
	//int wait_res = waitpid(curr_pid, &status, WNOHANG | WUNTRACED);
	//if(WIFEXITED(status))
	//{
		//we can print "smash> here .... "
		//cout<< "has exited"<<endl;
		//return;
	//}
	//if(wait_res != 0)//che child is not running!
	//{
	//	return;
	//}
    int kill_res = kill(curr_pid, SIGKILL);
    if(kill_res == -1)
    {
        perror("smash error: kill failed");
        return;
    }
    cout << "smash: process " << curr_pid << " was killed" << endl;
    
    //layan trying to pass last test:
    //smash.removeFinishedJobs();
    //smash.getJobsList()->removeJobByPid(curr_pid);
    return;
}

void ctrlZHandler(int sig_num)
{
    cout << "smash: got ctrl-Z" << endl;
    SmallShell& smash = SmallShell::getInstance();
    pid_t curr_pid = smash.getCurrPid();
	pid_t smash_pid = getpid();
    if(smash_pid == -1)
    {
		perror("smash error: getpid failed");
        return;
	}
	if(curr_pid == smash_pid)
	{
		return;
	}
    //if there's a process running in the foreground:
    if(!smash.isFGProcess())
    {
        return;
    }

    int kill_res = kill(curr_pid, SIGSTOP); // what pid??
    if(kill_res == -1)
    {
        perror("smash error: kill failed");
        return;
    }

    cout << "smash: process " << curr_pid << " was stopped" << endl;
	
    //should first check if this process is in the jobs list or not!

    JobsList::JobEntry* job_entry = smash.getJobsList()->getJobByPid(curr_pid);
    if(job_entry != nullptr)
    {
        job_entry->stop();
    }
    else
    {
        char* curr_cmd = smash.getCurrCmd();
        smash.getJobsList()->addJob(curr_cmd, curr_pid, true);
    }
    return;
}


