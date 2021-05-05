#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    SmallShell& sm = SmallShell::getInstance();
    JobsList& jobsList = sm.getJobsList();
    if (jobsList.currJobInFg || sm.getcurrCommandInFgPid() != -1) { // if there is a job running in the foreground (otherwise, nothing will happen, will ignore)
        pid_t pid;
        if (jobsList.currJobInFg) {
            pid = jobsList.currJobInFg->getProcessID();
        }
        else{
            pid = sm.getcurrCommandInFgPid();
        }
        int error_kill = kill(pid, SIGSTOP);
        if(error_kill == -1){
            perror("smash error: kill failed");
            return;
        }
        if(jobsList.currJobInFg){
            jobsList.currJobInFg->setStatus(STOPPED);
            jobsList.currJobInFg->setTime();
            jobsList.jobsMap.insert(std::pair<int, JobsList::JobEntry*>(jobsList.currJobInFg->getJobID(),
                                                                        jobsList.currJobInFg));//added job to jobList
            jobsList.currJobInFg = nullptr;
        }else{
            JobsList::JobEntry * newJobEntry = new JobsList::JobEntry(jobsList.nextID, pid, (string)sm.getcurrCommandInFgCmd(),time(NULL),STOPPED);
            jobsList.jobsMap.insert(std::pair<int,JobsList::JobEntry*>(jobsList.nextID, newJobEntry));//added the job to the job list
            jobsList.nextID++;
            sm.setCurrCommandInFgPid(-1);
            sm.setCurrCommandInFgCmd("");
        }
        cout << "smash: process " << pid << " was stopped" << endl;
    }
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    SmallShell& sm = SmallShell::getInstance();
    JobsList& jobsList = sm.getJobsList();
    if (jobsList.currJobInFg) { // if there is a job running in the foreground (otherwise, nothing will happen, will ignore)
        pid_t pid = jobsList.currJobInFg->getProcessID();
        int error_kill = kill(pid, SIGKILL);
        if(error_kill == -1){
            perror("smash error: kill failed");
            return;
        }
        jobsList.currJobInFg = nullptr;
        cout << "smash: process " << pid << " was killed" << endl;
    }
    if (!jobsList.jobsMap.empty()) {
        jobsList.nextID = (--jobsList.jobsMap.end())->first + 1;
    }else {
        jobsList.nextID = 1;
    }
}

void alarmHandler(int sig_num) {
    cout << "smash: got an alarm" << endl;
    SmallShell& sm = SmallShell::getInstance();
    JobsList& jobsList = sm.getJobsList();
    JobsList& timeoutList = sm.getTimeoutList();
    pid_t pidToAlarm;
    if(timeoutList.jobsMap.begin()->second != nullptr) {
        pidToAlarm = timeoutList.jobsMap.begin()->second->getProcessID();
        int error_kill = kill(pidToAlarm, SIGALRM);
        if(error_kill == -1){
            perror("smash error: kill failed");
            return;
        }
        cout << timeoutList.jobsMap.begin()->second->getCommand() << " timed out!" << endl;

        int jobId = timeoutList.jobsMap.begin()->second->getJobID();
        JobsList::JobEntry* toErase = timeoutList.getJobById(jobId);
        if(toErase != nullptr){
            if(toErase->getStatus() == STOPPED || toErase->getStatus() == BACKGROUND) {
                for (auto it = jobsList.jobsMap.begin(); it != jobsList.jobsMap.end(); ++it){ //removes the terminated process from the jobsList
                    if(it->second == toErase) {
                        delete jobsList.jobsMap.find(it->first)->second;///Added. Maybe there is no need
                        jobsList.jobsMap.erase(jobsList.jobsMap.find(it->first));
                    }

                }
            }
            if (!jobsList.jobsMap.empty()) {
                jobsList.nextID = (--jobsList.jobsMap.end())->first + 1;
            }else {
                jobsList.nextID = 1;
            }
        }
        delete timeoutList.jobsMap.find(timeoutList.jobsMap.begin()->first)->second;
        timeoutList.jobsMap.erase(timeoutList.jobsMap.begin());
    }
}

