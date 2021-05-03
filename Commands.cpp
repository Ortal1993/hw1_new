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
#include <fcntl.h>

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

bool _isPipeCommand(const char* cmd_line){
    int i = 0;
    string s = _trim(string(cmd_line));
    int size = s.size();
    while(i < size){
        if(s.at(i) == '|'){
            return true;
        }else{
            i++;
        }
    }
    return false;
}

bool _isRedirectionCommand(const char* cmd_line){
    int i = 0;
    string s = _trim(string(cmd_line));
    int size = s.size();
    while(i < size){
        if(s.at(i) == '>'){
            return true;
        }else{
            i++;
        }
    }
    return false;
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
    if(_isPipeCommand(cmd_line)){
        return new PipeCommand(cmd_line);
    }
    if(_isRedirectionCommand(cmd_line)){
        return new RedirectionCommand(cmd_line);
    }

    string copiedCmd = cmd_line;
    if(_isBackgroundComamnd(copiedCmd.c_str())){
        _removeBackgroundSign((char*)copiedCmd.c_str());
    }

    string cmd_s = _trim(string(copiedCmd));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(WHITESPACE));

    if(firstWord.compare("chprompt") == 0){
        return new ChangePromptCommand(cmd_line);
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
    else if (firstWord.compare("cat") == 0){
        return new CatCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    delete cmd;
}

std::string SmallShell::GetPrompt() {
    return this->prompt;
}

JobsList& SmallShell::getJobsList() {
    return this->jobsList;
}

void parsing(string& toParse, int& argLength, vector<string>& arguments, char c1, char c2){
    string s;
    if(toParse.at(argLength + 1) == c1){
        s += c1;
        s += c2;
        arguments.push_back(s);
        toParse = _ltrim(toParse.substr(argLength + 2));
        argLength = toParse.length();
        return;
    }
    s = c1;
    arguments.push_back(s);
    toParse = _ltrim(toParse.substr(argLength + 1));
    argLength = toParse.length();
}

Command::Command(const char *cmd_line): cmd_line(cmd_line), smInstance(SmallShell::getInstance()){
    string toParse = (string)this->cmd_line;
    toParse = _trim(toParse);
    int argLength = 0;
    while(argLength < (int)toParse.length() && (!(toParse.at(argLength) == '>' || toParse.at(argLength) == '|'))){
        argLength++;
    }
    string currArgument;
    while(toParse.length() > 0){//arguments[0] = command
        if(argLength == (int)toParse.length()){
            argLength = toParse.find_first_of(WHITESPACE);
            if (argLength == -1) {
                string currArgument = toParse.substr(0, argLength);
                this->arguments.push_back(currArgument);
                break;
            }
        }
        currArgument = toParse.substr(0, argLength);
        this->arguments.push_back(currArgument);
        if(toParse.at(argLength) == '>'){
            parsing(toParse, argLength, this->arguments, '>', '>');
            continue;
        }
        if(toParse.at(argLength) == '|'){
            parsing(toParse, argLength, this->arguments, '|', '&');
            continue;
        }
        toParse = _ltrim(toParse.substr(argLength));
        argLength = toParse.length();
    }
}

string Command::GetArgument(int argNum) {
    return this->arguments[argNum];
}

int Command::GetNumOfArgs() {
    return this->arguments.size();
}

RedirectionCommand::RedirectionCommand(const char* cmd_line): Command(cmd_line), fd(1) {
    for (int i = 0; i < this->GetNumOfArgs(); i++) {
        if (this->GetArgument(i) == ">" || this->GetArgument(i) == ">>") {
            if ((this->stdout_copy = dup(1)) == -1) {
                perror("smash error: dup failed");
                return;
            }
            if (close(1) == -1) {
                perror("smash error: close failed");
                return;
            }
            int newFd;
            if (this->GetArgument(i) == ">" || this->GetArgument(i) == ">>") {
                const char* fileName = this->GetArgument(i + 1).c_str();
                if(_isBackgroundComamnd(fileName)){
                    _removeBackgroundSign((char*)fileName);
                }
                if (this->GetArgument(i) == ">") {
                    newFd = open(fileName, O_RDWR | O_CREAT, 0666);
                    if (newFd == -1) {
                        perror("smash error: open failed");
                        return;
                    }
                } else {
                    newFd = open(this->GetArgument(i + 1).c_str(), O_APPEND | O_CREAT | O_RDWR, 0666);
                    if (newFd == -1) {
                        perror("smash error: open failed");
                        return;
                    }
                }
                this->fd = newFd;
                break;
            }
        }else {
            this->leftCommand.push_back(this->GetArgument(i));
        }
    }
}


RedirectionCommand::~RedirectionCommand() {
    if (dup2(this->stdout_copy, this->fd) == -1) {
        perror("smash error: dup2 failed");
        return;
    }
}

void RedirectionCommand::execute() {
    std::string leftCommand = "";
    for(int i = 0; i < (int)this->getLeftCommand().size(); i++){
        leftCommand += this->getLeftCommand()[i];
        leftCommand += " ";
    }
    this->getSmallShell().executeCommand(leftCommand.c_str());
}

void removeFinishedJobs(SmallShell& sm){
    JobsList& jobs = sm.getJobsList();
    std::vector<int> jobsToDelete;
    pid_t status = 0;
    for(auto it = jobs.jobsMap.begin(); it != jobs.jobsMap.end(); ++it){
        status = waitpid(it->second->getProcessID(), nullptr, WNOHANG);
        if (status == -1) {
            perror("smash error: waitpid failed");
            return;
        }
        if (status != 0) { //this process is zombie (terminated), need to be removed from jobs
            jobsToDelete.push_back(it->first);
        }
    }
    for (auto it = jobsToDelete.begin(); it != jobsToDelete.end(); ++it){ //removes the terminated process from the jobsList
        delete jobs.jobsMap.find(*it)->second;///Added. Maybe there is no need
        jobs.jobsMap.erase(jobs.jobsMap.find(*it));
    }
    if (!jobs.jobsMap.empty()) {
        jobs.nextID = (--jobs.jobsMap.end())->first + 1;
    }else {
        jobs.nextID = 1;
    }
}

void ExternalCommand::execute(){
    SmallShell& sm = getSmallShell();
    pid_t pid = fork();
    if(pid == -1){
        perror("smash error: fork failed");
        return;
    }
    if(pid == 0) {// child
        if(setpgrp() == -1){
            perror("smash error: setpgrp failed");
            return;
        }
        const char* originalCmd = getCmd();
        _removeBackgroundSign((char *)originalCmd);
        char* argv[] = {(char*)"/bin/bash", (char*)"-c", (char*) originalCmd, NULL};
        int error_execv = execv(argv[0], argv);
        if(error_execv == -1){
            perror("smash error: execv failed");
            return;
        }
        exit(0);
    }
    else{//father
        int lastArgument = this->GetNumOfArgs();
        string str = this->GetArgument(lastArgument - 1);
        if((str == "&") | _isBackgroundComamnd(str.c_str())){//should run in background
            removeFinishedJobs(sm); //removing all finished jobs before adding a new one
            JobsList& jobs = getSmallShell().getJobsList();
            JobsList::JobEntry * newJobEntry = new JobsList::JobEntry(jobs.nextID, pid, (string)getCmd(),time(NULL),BACKGROUND);
            jobs.jobsMap.insert(std::pair<int,JobsList::JobEntry*>(jobs.nextID, newJobEntry));//added the job to the job list
            jobs.nextID++;
        }else{//should run in foreground
            int error_waitpid = waitpid(pid,NULL, WUNTRACED);
            if(error_waitpid == -1){
                perror("smash error: waitpid failed");
                return;
            }
        }
    }
}

///func 1 - showpid
void ShowPidCommand::execute() {
    std::cout << "smash pid is: " << getSmallShell().getPid() << endl;
}

///func 2 - chprompt
void ChangePromptCommand::execute(){
    int numOfArgs = this->GetNumOfArgs();
    SmallShell& sm = getSmallShell();
    if (numOfArgs == 1){//arguments[0] = command
       sm.setPrompt("smash");
    }else{//numOfArgs > 1
        sm.setPrompt(this->GetArgument(1));
    }
}

///func 3 - pwd
void GetCurrDirCommand::execute() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << cwd << endl;
    } else {
        perror("smash error: getcwd failed");
        return;
    }
}

///func 4 - cd
void ChangeDirCommand::execute() {
    int numOfArgs = this->GetNumOfArgs();
    SmallShell& sm = getSmallShell();
    string lastPwd = sm.getLastPwd();
    string currentPwd = sm.getCurrentPwd();
    if (numOfArgs > 2){//print error
        std::cerr << "smash error: cd: too many arguments" << endl;
    }else if (numOfArgs == 2){
        string newPath = this->GetArgument(1);
        if (newPath == "-" && lastPwd != ""){
            const char * path = lastPwd.c_str();
            int error = chdir(path);
            if(error == -1){//syscall failed
                perror("smash error: chdir failed");
                return;
            }
            else{
                sm.setLastPwd(currentPwd);
                sm.setCurrentPwd(lastPwd);
            }
        } else if (newPath == "-" && lastPwd == ""){
            std::cerr << "smash error: cd: OLDPWD not set"  << endl;
        } else if(newPath == ".."){
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                string currPath = cwd;
                string new_path = currPath;
                unsigned int idx = new_path.find_last_of('/');
                if(idx != 0){
                    new_path = new_path.substr(0, idx);
                }else{
                    new_path = new_path.substr(0, 1);
                }
                const char * path = new_path.c_str();
                int error = chdir(path);
                if(error == -1){//syscall failed
                    perror("smash error: chdir failed");
                    return;
                }else{
                    sm.setLastPwd(currPath);
                    sm.setCurrentPwd(path);
                }
            } else {
                perror("smash error: getcwd failed");
                return;
            }
        }else {//a regular path is given
            const char * path = newPath.c_str();
            int error = chdir(path);
            if(error == -1){//syscall failed
                perror("smash error: chdir failed");
                return;
            }
            else{
                sm.setLastPwd(currentPwd);
                sm.setCurrentPwd(path);
            }
        }
    }
}

void printJobs(SmallShell& sm){
    JobsList& jobs = sm.getJobsList();
    for (auto it = jobs.jobsMap.begin(); it != jobs.jobsMap.end(); ++it){ //removes the terminated process from the jobsList
        if (it->second->getStatus() == STOPPED){
            cout << "[" << it->first << "] " << it->second->getCommand() << ": " << it->second->getProcessID() << " " << std::abs(difftime(it->second->getTime(), time(NULL))) << " secs" << " (stopped)" << endl;
        } else {
            cout << "[" << it->first << "] " << it->second->getCommand() << ": " << it->second->getProcessID() << " " << std::abs(difftime(time(NULL), it->second->getTime())) << " secs" << endl;
        }
    }
}

///func 5 - jobs
void JobsCommand::execute(){
    SmallShell& sm = getSmallShell();
    removeFinishedJobs(sm);
    printJobs(sm);
}

///func 6 - kill
void KillCommand::execute() {
    int numOfArgs = this->GetNumOfArgs();
    int jobId;
    if (numOfArgs != 3) {//arguments[0] = command
        cerr << "smash error: kill: \"invalid arguments\"" << endl;
    }else{
        string sigNumStr = GetArgument(1);
        int i = 0;
        while(sigNumStr.at(i) == '-'){
            i++;
        }
        if(i == 0 || i > 1) {
            cerr << "smash error: kill: \"invalid arguments\"" << endl;
        }else if(i == 1){
            sigNumStr = sigNumStr.substr(1);
            int signum = 0;
            try{
                signum = stoi(sigNumStr);
            }
            catch (const std::invalid_argument& ia){
                cerr << "smash error: fg: \"invalid arguments\"" << endl;
                return;
            }
            string jobIdStr = GetArgument(2);
            try{
                jobId = stoi(jobIdStr);
            }
            catch (const std::invalid_argument& ia){
                cerr << "smash error: fg: \"invalid arguments\"" << endl;
                return;
            }
            if(getSmallShell().getJobsList().getJobById(jobId) != nullptr){
                int processID = getSmallShell().getJobsList().getJobById(jobId)->getProcessID();
                cout << "signal number " << signum << " was sent to pid " << processID << endl;
                int error = kill(processID, signum);
                if(error == -1){
                    perror("smash error: kill failed");
                    return;
                }
            }
            else {
                cerr << "smash error: kill: \"job-id " << jobId << " does not exist\"" << endl;
            }
        }
    }

}

///func 7 - fg
void ForegroundCommand::execute() {
    int jobToFg;
    JobsList& jobs = getSmallShell().getJobsList();
    if (GetNumOfArgs() > 2){
        cerr << "smash error: fg: \"invalid arguments\"" << endl;
        return;
    } else if (GetNumOfArgs() == 1){//if no jobId was specified
        jobToFg = (--jobs.jobsMap.end())->first;
    } else if (GetNumOfArgs() == 2){
        try {
            jobToFg = stoi(this->GetArgument(1));
        }
        catch (const std::invalid_argument& ia){
            cerr << "smash error: fg: \"invalid arguments\"" << endl;
            return;
        }
    }
    JobsList::JobEntry * jobEntry = jobs.getJobById(jobToFg);
    if (!jobEntry && GetNumOfArgs() == 2){//job was not found, no jobID in jobList
        cerr << "smash error: fg: \"job-id " << jobToFg << " does not exist\"" << endl;
        return;
    }
    else if (!jobEntry && GetNumOfArgs() == 1){
        cerr << "smash error: fg: \"jobs list is empty\"" << endl;
        return;
    }
    else {//job was found
        pid_t pidToFg = jobEntry->getProcessID();
        if (jobEntry->getStatus() == STOPPED) {
            int error_kill = kill(pidToFg, SIGCONT);
            if(error_kill == -1){
                perror("smash error: kill failed");
                return;
            }
        }
        cout << jobEntry->getCommand() << " : " << pidToFg << endl;
        jobs.currJobInFg = new JobsList::JobEntry(*jobEntry);//copy constructor
        delete jobs.getJobById(pidToFg);///Added. Maybe there is no need
        jobs.jobsMap.erase(jobToFg);//Iterators, pointers and references referring to elements removed by the function are invalidated.
        int error_waitpid = waitpid(pidToFg, NULL, WUNTRACED);//WUNTRACED: also return if a child has stopped
        if(error_waitpid == -1){
            perror("smash error: kill failed");
            return;
        }
        delete jobs.currJobInFg;
    }
}

int findMaxJobIDbyStatus(SmallShell& sm, STATUS status){
    int maxLastStoppedJobId = -1;
    for(map<int, JobsList::JobEntry*>::iterator it = sm.getJobsList().jobsMap.begin(); it != sm.getJobsList().jobsMap.end(); ++it){
        if(it->second->getStatus() == status){
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
        int maxLastStoppedJobId = findMaxJobIDbyStatus(sm, STOPPED);
        if(maxLastStoppedJobId == -1){//
            cerr << "smash error: bg: \"there is no stopped jobs to resume\"" << endl;
        }else{
            JobsList::JobEntry* stoppedJobToBg = sm.getJobsList().getJobById(maxLastStoppedJobId);
            string cmdLine = stoppedJobToBg->getCommand();
            cout << cmdLine << endl;
            int error_kill = kill(stoppedJobToBg->getProcessID(), SIGCONT);
            if(error_kill == -1){
                perror("smash error: kill failed");
                return;
            }
            stoppedJobToBg->setStatus(BACKGROUND);
        }
    }else if(numOfArgs == 2){
        std::string jobIdStr = this->GetArgument(1);
        int jobID;
        try{
            jobID = stoi(jobIdStr);
        }catch(const std::invalid_argument& ia){
            cerr << "smash error: bg: \"invalid arguments\"" << endl;
        }
        JobsList::JobEntry* spcStoppedJobToBg = sm.getJobsList().getJobById(jobID);
        if(spcStoppedJobToBg != nullptr){ //specific job-id
            if(spcStoppedJobToBg->getStatus() == STOPPED){
                string spcCmdLine = spcStoppedJobToBg->getCommand();
                cout << spcCmdLine << endl;
                kill(spcStoppedJobToBg->getProcessID(), SIGCONT);
                spcStoppedJobToBg->setStatus(BACKGROUND);
            }else{
                cerr << "smash error: bg: \"job-id " << jobID << " is already running in the background\"" << endl;
            }
        }else{
            cerr << "smash error: bg: \"job-id" << jobID << "does not exist\"" << endl;
        }
    }
}

///func 9 - quit
void QuitCommand::execute() {
    int numOfArgs = this->GetNumOfArgs();
    if(numOfArgs == 1 || (numOfArgs > 1 && this->GetArgument(1) != "kill")){
        exit(0xff);///???
    }
    if(numOfArgs > 1 && this->GetArgument(1) == "kill"){
        SmallShell& sm = getSmallShell();
        removeFinishedJobs(sm); //remove finished jobs
        int numJobs = sm.getJobsList().jobsMap.size();
        cout << "smash: sending SIGKILL signal to " << numJobs << " jobs:" << endl;
        for(map<int, JobsList::JobEntry*>::iterator it = sm.getJobsList().jobsMap.begin(); it != sm.getJobsList().jobsMap.end(); ++it){
            int currJobPid = it->second->getProcessID();
            cout << currJobPid << ": " << it->second->getCommand() << endl;
            int error_kill = kill(currJobPid, 9);///does it delete the *jobEntry?
            if(error_kill == -1){
                perror("smash error: kill failed");
                return;
            }
        }
        exit(0xff);///???
    }
}

///pipe
PipeCommand::PipeCommand(const char *cmd_line):Command(cmd_line) {
    for(int i = 0; i < this->GetNumOfArgs(); i++) {
        if (this->GetArgument(i) == "|" || this->GetArgument(i) == "|&") {
            if(this->GetArgument(i) == "|"){
                this->out = COUT;
            }else{
                this->out = CERR;
            }
            i++;
            while(i < GetNumOfArgs()){
                const char* curr = this->GetArgument(i).c_str();//pipe cannot be done on background command.
                if(_isBackgroundComamnd(curr)){
                    _removeBackgroundSign((char*)curr);
                }
                this->rightCommand.push_back(this->GetArgument(i++));
            }
            break;
        }
        const char* curr = this->GetArgument(i).c_str();//pipe cannot be done on background command.
        if(_isBackgroundComamnd(curr)){
            _removeBackgroundSign((char*)curr);
        }
        this->leftCommand.push_back((string)curr);
    }
}

void PipeCommand::execute() {
    int my_pipe[2];
    int error_pipe = pipe(my_pipe);
    if(error_pipe == -1){
        perror("smash error: pipe failed");
        return;
    }
    int p1;
    int p2;
    if((p1 = fork()) == 0){//child
        int error_setpgrp = setpgrp();
        if(error_setpgrp == -1){
            perror("smash error: setpgrp failed");
            return;
        }
        int error_dup2 = dup2(my_pipe[1], this->getOut());
        if(error_dup2 == -1){
            perror("smash error: dup2 failed");
            return;
        }
        int error_close1 = close(my_pipe[1]);
        if(error_close1 == -1){
            perror("smash error: close failed");
            return;
        }
        int error_close0 = close(my_pipe[0]);
        if(error_close0 == -1){
            perror("smash error: close failed");
            return;
        }
        std::string left = "";
        for(int i = 0; i < (int)this->getLeft().size(); i++){
            left += this->getLeft()[i];
            left += " ";
        }
        this->getSmallShell().executeCommand(left.c_str());
        exit(0);
    }if((p2 = fork()) == 0){//second child
        int error_setpgrp = setpgrp();
        if(error_setpgrp == -1){
            perror("smash error: setpgrp failed");
            return;
        }
        int error_dup2 = dup2(my_pipe[0], 0);
        if(error_dup2 == -1){
            perror("smash error: dup2 failed");
            return;
        }
        int error_close0 = close(my_pipe[0]);
        if(error_close0 == -1){
            perror("smash error: close failed");
            return;
        }
        int error_close1 = close(my_pipe[1]);
        if(error_close1 == -1){
            perror("smash error: close failed");
            return;
        }
        std::string right = "";
        for(int i = 0; i < (int)this->getRight().size(); i++){
            right += this->getRight()[i];
            right += " ";
        }
        this->getSmallShell().executeCommand(right.c_str());
        exit(0);
    };
    if(p1 == -1 || p2 == -1){
        perror("smash error: fork failed");
        return;
    }
    int close0 = close(my_pipe[0]);
    if(close0 == -1){
        perror("smash error: close failed");
        return;
    }
    int close1 = close(my_pipe[1]);
    if(close1 == -1){
        perror("smash error: close failed");
        return;
    }
    int error_waitpid1 = waitpid(p1, NULL, 0);
    if(error_waitpid1 == -1){
        perror("smash error: waitpid failed");
        return;
    }
    int error_waitpid2 = waitpid(p2, NULL, 0);
    if(error_waitpid2 == -1){
        perror("smash error: waitpid failed");
        return;
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    if(this->jobsMap.find(jobId) != jobsMap.end()){
        return this->jobsMap.find(jobId)->second;
    }else{
        return nullptr;
    }
}

pid_t JobsList::JobEntry::getProcessID() {
    return this->processID;
}

std::string JobsList::JobEntry::getCommand() {
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

void CatCommand::execute() {
    for (int i = 1; i < this->GetNumOfArgs(); i++) {
        int readNum = 0;
        int fd = open(this->GetArgument(i).c_str(), O_RDONLY , 0666);
        if (fd == -1) {
            perror("smash error: open failed");
            break;
        }
        do { //opened successfully
            readNum = read(fd, this->buf, 256);
            if (readNum == -1) {
                perror("smash error: read failed");
                return;
            }
            int writeNum;
            writeNum = write(1, this->buf, readNum);
            if (writeNum == -1) {
                perror("smash error: write failed");
                return;
            }
            delete (this->buf);
            this->buf = new char[256];
        } while (readNum > 0);
        int error = close(fd);
        if(error == -1) {
            perror("smash error: close failed");
            break;
        }
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

/*const char* SmashExceptions::what() const noexcept{
    return what_message.c_str();
}

KillInvalidArg::KillInvalidArg() : SmashExceptions("smash error: kill: invalid arguments"){}*/