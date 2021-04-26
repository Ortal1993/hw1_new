#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <limits.h>
#include <utility>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const std::string WHITESPACE = " \n\r\t\f\v";


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

bool _isBackgroundComamnd(const char* cmd_line) {
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

//SmallShell::SmallShell() {
// TODO: add your implementation
//}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
/*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(WHITESPACE));///originally \n

  if(firstWord.compare("chprompt") == 0){
      return new RedirectionCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
      return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
      return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0){
      return new JobsCommand(cmd_line);
  }
  else if (firstWord.compare("kill") == 0){
      return new KillCommand(cmd_line);
  }
  else if (firstWord.compare("fg") == 0){
      return new ForegroundCommand(cmd_line);
  }
  else if (firstWord.compare("bg") == 0){
      return new BackgroundCommand(cmd_line);
  }
  else if (firstWord.compare("quit") == 0){
      return new QuitCommand(cmd_line);
  }
  else {
      return new ExternalCommand(cmd_line);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    ///delete cmd;
}

std::string SmallShell::GetPrompt() {
    return this->prompt;
}

JobsList& SmallShell::getJobsList() {
    return this->jobsList;
}

Command::Command(const char *cmd_line): cmd_line(cmd_line), sms(SmallShell::getInstance()){
    //this->sms = SmallShell::getInstance();

    string toParse = (string)this->cmd_line;
    toParse = _trim(toParse);
    while(toParse.length() > 0){///arguments[0] = command
        int argLength = toParse.find_first_of(WHITESPACE);
        if(argLength == -1){
            string currArgument = toParse.substr(0, argLength);
            this->arguments.push_back(currArgument);
            break;
        }
        string currArgument = toParse.substr(0, argLength);
        this->arguments.push_back(currArgument);
        toParse = toParse.substr(argLength + 1);
        toParse = _ltrim(toParse);
    }
}

string Command::GetArgument(int argNum) {
    /*if (argNum < 1 || argNum > this->GetNumOfArgs()){///arguments[0] = command
        return "";
    }*/
    return this->arguments[argNum];
}

int Command::GetNumOfArgs() {
    return this->arguments.size();
}

void ExternalCommand::execute(){
    SmallShell& sm = getSmallShell();
    pid_t pid = fork();
    if( pid == 0 ) { // child process code goes here
        setpgrp();
        string originalCmd = (string)getCmd();
        char * args = new char(sizeof (originalCmd.size()));
        strcpy(args,getCmd());
        char* argv[] = {(char*)"/bin/bash", (char*)"-c", args, NULL};
        execv(argv[0], argv);

        /*sm.getJobsList().currJobInFg = pid;///???
        cout << "pid child is:" << sm.getJobsList().currJobInFg << endl;*/
        ///delete args?
    }
    else{//father
        int lastArgument = this->GetNumOfArgs();
        string str = this->GetArgument(lastArgument - 1);///what if there are many spaces before &
        if (str != "&" && str[str.size()-1] != '&'){//if should run in foreground
            waitpid(pid,NULL, WUNTRACED);///THE THIRD ARG WAS NULL - I CHANGED TO WUNTRACED
        }
        else{//should run in background
            JobsList& jobs = getSmallShell().getJobsList();
            JobsList::JobEntry * newJobEntry = new JobsList::JobEntry(pid, (string)getCmd(),time(NULL),BACKGROUND);
            int newJobId = jobs.nextID;
            jobs.nextID++;
            jobs.jobsMap.insert(std::pair<int,JobsList::JobEntry*>(newJobId,newJobEntry));//added the job to the job list
        }
    }

}

///func 1 - showpid
void ShowPidCommand::execute() {
    std::cout << "smash pid is: " << getSmallShell().getPid() << endl;
}

///func 2 - chprompt
void RedirectionCommand::execute(){
    int numOfArgs = this->GetNumOfArgs();
    SmallShell& sm = getSmallShell();
    if (numOfArgs == 1){///arguments[0] = command
       sm.setPrompt("smash");
    }else{///numOfArgs > 1
        sm.setPrompt(this->GetArgument(1));
    }
}

///func 3 - pwd
void GetCurrDirCommand::execute() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working dir: " << cwd << endl;
    } else {
        std::cout << "error: "  << endl;
    }
}

///func 4 - cd
void ChangeDirCommand::execute() {
    int numOfArgs = this->GetNumOfArgs();
    SmallShell& sm = getSmallShell();
    string lastPwd = sm.getLastPwd();
    string currentPwd = sm.getCurrentPwd();
    if (numOfArgs > 2){
        //print error
        std::cerr << "smash error: cd: too many arguments" << endl;
    }else if (numOfArgs == 2){
        string arg = this->GetArgument(1);
        if (arg == "-" && lastPwd != ""){
            const char * path = lastPwd.c_str();
            int error = chdir(path);
            if(error == -1){//syscall failed
                //print error, errno
                perror("smash error: chdir failed");
            }
            else{
                sm.setLastPwd(currentPwd);
                sm.setCurrentPwd(lastPwd);
            }
        } else if (arg == "-" && lastPwd == ""){
            //print error no OLDPWD
            std::cerr << "smash error: cd: OLDPWD not set"  << endl;
        } else {
            const char * path = arg.c_str();
            int error = chdir(path);
            if(error == -1){//syscall failedl
                //print error, errno
                //std::cout << "error: sysCallFail2"  << endl;
                perror("smash error: chdir failed");
            }
            else{
                if (currentPwd == ""){//if *currentPwd is null, then it is the first time doing cd
                    sm.setLastPwd("");
                }
                else{
                    sm.setLastPwd(currentPwd);
                }
                sm.setCurrentPwd(const_cast<char *>(path));
            }
        }
    }
}

///func 5 - jobs
void JobsCommand::execute(){
    JobsList& jobs = getSmallShell().getJobsList();
    std::vector<int> vec;
    for(auto it = jobs.jobsMap.begin(); it != jobs.jobsMap.end(); ++it){
        int status = waitpid(it->second->GetProcessID(),NULL,WNOHANG);
        if (status > 0) { //this process is zombie (terminated), need to remove from jobs
            vec.push_back(it->first);
        }else{
            if (it->second->getStatus() == STOPPED){
                cout << "[" << it->first << "] " << it->second->GetCommand() << ": " << it->second->GetProcessID() << " " << difftime(it->second->getTime(), time(NULL)) << " (stopped)" <<endl;
            } else {
                cout << "[" << it->first << "] " << it->second->GetCommand() << ": " << it->second->GetProcessID() << " " << difftime(it->second->getTime(), time(NULL)) <<endl;
            }
        }
    }
    for (auto it = vec.begin(); it != vec.end(); ++it){ //removes the terminated process from the jobsList
        jobs.jobsMap.erase(jobs.jobsMap.find(*it));
    }
}

///func 6 - kill
void KillCommand::execute() {
    int numOfArgs = this->GetNumOfArgs();
    if (numOfArgs != 3) {///arguments[0] = command ///if we get: kill -  9 7 - is it legal?
        cerr << "smash error: kill: invalid arguments" << endl;
    }else{
        string signumStr = GetArgument(1);
        int i = 0;
        while(signumStr.at(i) == '-'){
            i++;
        }
        if(i == 0 || i > 1) {
            cerr << "smash error: kill: invalid arguments" << endl;
        }else if(i == 1){
            signumStr = signumStr.substr(1);
            int signum = stoi(signumStr);
            string jobIdStr = GetArgument(2);
            int jobId = stoi(jobIdStr); ///std::invalid_argument
            if(getSmallShell().getJobsList().getJobById(jobId) != nullptr){
                int processID = getSmallShell().getJobsList().getJobById(jobId)->GetProcessID();
                cout << "signal number " << signum << "was sent to pid" << processID;
                int error = kill(processID, signum);
                if(error == -1){
                    perror("smash error: kill failed");
                }
            }
        }
    }

}

///func 7 - fg
void ForegroundCommand::execute() {
    int jobToFg;
    JobsList& jobs = getSmallShell().getJobsList();
    if (GetNumOfArgs() > 2){
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    } else if (GetNumOfArgs() == 1){//if no jobId was specified
        jobToFg = jobs.jobsMap.end()->first;
    } else if (GetNumOfArgs() == 2){
        try {
            jobToFg = stoi(this->GetArgument(1));
        }
        catch (const std::invalid_argument& ia){
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
    }
    JobsList::JobEntry * jobEntry = jobs.getJobById(jobToFg);
    if (!jobEntry && GetNumOfArgs() == 2){//job was not found, no jobID in jobList
        cerr << "smash error: fg: job-id " << jobToFg << " does not exist" << endl;
        return;
    }
    else if (!jobEntry && GetNumOfArgs() == 1){
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    else {
        pid_t pidToFg = jobEntry->GetProcessID();
        if (jobEntry->getStatus() == STOPPED) {
            kill(pidToFg, SIGCONT);
        }
        cout << jobEntry->GetCommand() << " : " << pidToFg << endl;
        *jobs.currJobInFg = JobsList::JobEntry(*jobEntry);//copy constructor
        jobs.jobsMap.erase(jobToFg);//Iterators, pointers and references referring to elements removed by the function are invalidated.
        waitpid(pidToFg, NULL, WUNTRACED);//WUNTRACED: also return if a child has stopped
        //getSmallShell().getJobsList().currJobInFg = -1;
    }
}

int findMaxJobIDbyStatus(SmallShell& sm, STATUS status){
    int maxLastStoppedJobId = -1;
    for(map<int, JobsList::JobEntry*>::iterator it = sm.getJobsList().jobsMap.begin(); it != sm.getJobsList().jobsMap.end(); ++it){
        if(it->second->getStatus() == STOPPED){
            if(it->first > maxLastStoppedJobId){
                maxLastStoppedJobId = it->first;
            }
        }
    }
    return maxLastStoppedJobId;
}

///func 8 - bg
void BackgroundCommand::execute() {
    int numOfArgs = this->GetNumOfArgs();
    SmallShell& sm = getSmallShell();
    if(numOfArgs == 1){//no job-id was given, take the last stopped job with the maximal job-id
        int lastStoppedJobID = sm.getJobsList().lastStoppedJobID;
        if(lastStoppedJobID == -1){
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
        }else{
            JobsList::JobEntry* stoppedJobToBg = sm.getJobsList().getJobById(lastStoppedJobID);
            string cmdLine = stoppedJobToBg->GetCommand();
            cout << cmdLine << endl;
            kill(stoppedJobToBg->GetProcessID(), SIGCONT);
            stoppedJobToBg->setStatus(BACKGROUND);
            int maxLastStoppedJobId = findMaxJobIDbyStatus(sm, STOPPED);
            sm.getJobsList().lastStoppedJobID = maxLastStoppedJobId;
        }
    }else if(numOfArgs == 2){
        std::string jobIdStr = this->GetArgument(1);
        int jobID;
        try{
            jobID = stoi(jobIdStr);
        }catch(const std::invalid_argument& ia){
            cerr << "smash error: bg: invalid arguments" << endl;
        }
        JobsList::JobEntry* spcStoppedJobToBg = sm.getJobsList().getJobById(jobID);
        if(spcStoppedJobToBg != nullptr){ //specific job-id
            if(spcStoppedJobToBg->getStatus() != STOPPED){
                string spcCmdLine = spcStoppedJobToBg->GetCommand();
                cout << spcCmdLine << endl;
                kill(spcStoppedJobToBg->GetProcessID(), SIGCONT);
                spcStoppedJobToBg->setStatus(BACKGROUND);
                int maxLastStoppedJobId1 = findMaxJobIDbyStatus(sm, STOPPED);
                sm.getJobsList().lastStoppedJobID = maxLastStoppedJobId1;
            }else{
                cerr << "smash error: bg: job-id" << jobID << "is already running in the background" << endl;
            }
        }else{
            cerr << "smash error: bg: job-id" << jobID << "does not exist" << endl;
        }
    }
}

///func 9 - quit
void QuitCommand::execute() {
    int numOfArgs = this->GetNumOfArgs();
    if(numOfArgs == 1 || numOfArgs > 1 && this->GetArgument(1) != "kill"){
        exit(0xff);///???
    }
    if(numOfArgs > 1 && this->GetArgument(1) == "kill"){
        SmallShell& sm = getSmallShell();
        int numJobs = sm.getJobsList().jobsMap.size();
        cout << "smash: sending SIGKILL signal to " << numJobs << " jobs:" << endl;
        for(map<int, JobsList::JobEntry*>::iterator it = sm.getJobsList().jobsMap.begin(); it != sm.getJobsList().jobsMap.end(); ++it){
            int currJobPid = it->second->GetProcessID();
            cout << currJobPid << ": " << it->second->GetCommand() << endl;
            kill(currJobPid, 9);
        }
    }
}

/*const char* SmashExceptions::what() const noexcept{
    return what_message.c_str();
}

KillInvalidArg::KillInvalidArg() : SmashExceptions("smash error: kill: invalid arguments"){}*/

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    if(this->jobsMap.find(jobId)->first != jobsMap.end()->first){
        return this->jobsMap.find(jobId)->second;
    }else{
        return nullptr;
    }
}

/*JobsList::JobEntry *JobsList::getLastStoppedJob(int jobId) {///in original passing pointer to int
    return this->jobsMap.find(jobId)->second;
}*/

/*int JobsList::getLastStoppedJobID() {
    return this->lastStoppedJobID;
}*/

/*void JobsList::setLastStoppedJobID(int maxJobId) {
    this->lastStoppedJobID = maxJobId;
}*/

int JobsList::JobEntry::GetProcessID() {
    return this->processID;
}

std::string JobsList::JobEntry::GetCommand() {
    return this->command;
}

STATUS JobsList::JobEntry::getStatus(){
    return this->status;
}

time_t JobsList::JobEntry:: getTime(){
    return this->enterTime;
}

void JobsList::JobEntry::setStatus(STATUS newStatus) {
    this->status = newStatus;
}

int JobsList::JobEntry::getJobID() {
    return this->jobID;
}

void JobsList::JobEntry::setTime(){
    this->enterTime = time(NULL);
}
