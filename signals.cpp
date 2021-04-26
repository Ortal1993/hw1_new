#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    SmallShell& sm = SmallShell::getInstance();
    JobsList& jobsList = sm.getJobsList();
    if (jobsList.currJobInFg) { // if there is a job running in the foreground (otherwise, nothing will happen, will ignore)
        jobsList.currJobInFg->setStatus(STOPPED);
        jobsList.currJobInFg->setTime();
        jobsList.jobsMap.insert(std::pair<int,JobsList::JobEntry*>(jobsList.currJobInFg->getJobID(), jobsList.currJobInFg));//added job to jobList
        pid_t p = jobsList.currJobInFg->GetProcessID();
        kill(p, SIGSTOP);
        jobsList.currJobInFg = nullptr;
        cout << "smash: process " << p << " was stopped" << endl;
    }
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    SmallShell& sm = SmallShell::getInstance();
    JobsList& jobsList = sm.getJobsList();
    if (jobsList.currJobInFg) { // if there is a job running in the foreground (otherwise, nothing will happen, will ignore)
        pid_t pid = jobsList.currJobInFg->GetProcessID();
        kill(pid, SIGKILL);
        jobsList.currJobInFg = nullptr;
        cout << "smash: process " << pid << " was killed" << endl;
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

