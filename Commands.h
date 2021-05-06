#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <unistd.h>
#include <map>
#include <ctime>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

enum STATUS{
    FOREGROUND = 0,
    BACKGROUND = 1,
    STOPPED = 2
};

enum OUTPUT{
    COUT = 1,
    CERR = 2
};

class SmallShell;

class Command{
    private:
        const char* cmd_line;
        std::vector<std::string> arguments;
        SmallShell& smInstance;
    public:
        Command(const char* cmd_line);
        virtual ~Command() {};
        virtual void execute() = 0;
        std::string GetArgument(int argNum);
        int GetNumOfArgs();
        SmallShell& getSmallShell() {return this->smInstance;}
        const char* getCmd (){return this->cmd_line;}
};

class BuiltInCommand : public Command {
    public:
        BuiltInCommand(const char* cmd_line): Command(cmd_line){}
        virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
        STATUS status;
    public:
        ExternalCommand(const char* cmd_line) : Command(cmd_line){};
        virtual ~ExternalCommand() {}
        void execute() override;
        STATUS getStatus() {return status;};
};

class RedirectionCommand : public Command {
    private:
        int stdout_copy;
        std::vector<std::string> leftCommand;
    public:
        RedirectionCommand(const char* cmd_line): Command(cmd_line){};
        virtual ~RedirectionCommand();
        void execute() override;
        std::vector<std::string>& getLeftCommand() {return this->leftCommand;};
};

///func 1 - chprompt
class ChangePromptCommand : public Command {///why inherits from Command?
    public:
        explicit ChangePromptCommand(const char* cmd_line): Command(cmd_line){};//, ptrPrompt(ptrPrompt){};
        virtual ~ChangePromptCommand() {}
        void execute() override;
};

///func 2 - showpid
class ShowPidCommand : public BuiltInCommand {
    public:
        ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
        virtual ~ShowPidCommand() = default;
        void execute() override;
};

///func 3 - pwd
class GetCurrDirCommand : public BuiltInCommand {
    public:
        GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
        virtual ~GetCurrDirCommand() {}
        void execute() override;
};

///func 4 - cd
class ChangeDirCommand : public BuiltInCommand {
    public:
        ChangeDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
        virtual ~ChangeDirCommand() {}
        void execute() override;
};

class JobsList {
    public:
        class JobEntry {
            private:
                int jobID;
                pid_t processID;
                std::string command;
                time_t enterTime;
                STATUS status;
            public:
                JobEntry(int jobID, pid_t processID, std::string command, time_t enterTime, STATUS status = BACKGROUND): jobID(jobID), processID(processID), command(command), enterTime(enterTime), status(status){};
                JobEntry(const JobEntry& jobEntry) = default;
                ~JobEntry() = default;
                pid_t getProcessID();
                std::string getCommand();
                STATUS getStatus();
                void setStatus(STATUS newStatus);
                time_t getTime();
                int getJobID();
                void setTime();
        };
        std::map<int, JobEntry*> jobsMap;//jobsList - the key is jobID //timeoutList -the key is timeForAlarm
        JobEntry* currJobInFg;
        int nextID;
    public:
        JobsList():jobsMap(), currJobInFg(nullptr), nextID(1){};//if lastStoppedJobID is -1 than no process has been stopped
        ~JobsList();
        //void addJob(Command* cmd, bool isStopped = false);
        //void printJobsList();
        //void killAllJobs();
        //void removeFinishedJobs();
        JobEntry* getJobById(int jobId);
        //void removeJobById(int jobId);
        //JobEntry* getLastJob(int* lastJobId);
};

///func 5 - jobs
class JobsCommand : public BuiltInCommand {
        ///When we declare a member of a class as static it means no matter how many objects of the class are created, there is only one copy of the static member.
    public:
        JobsCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
        virtual ~JobsCommand() {}
        void execute() override;
};

///func 6 - kill
class KillCommand : public BuiltInCommand {
    public:
        KillCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
        virtual ~KillCommand() {}
        void execute() override;
};

///func 7 - fg
class ForegroundCommand : public BuiltInCommand {
    public:
        ForegroundCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
        virtual ~ForegroundCommand() {}
        void execute() override;
};

///func 8 - bg
class BackgroundCommand : public BuiltInCommand {
    public:
        BackgroundCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
        virtual ~BackgroundCommand() {}
        void execute() override;
};

///func 9 - quit
class QuitCommand : public BuiltInCommand {

    public:
        QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
        virtual ~QuitCommand() {}
        void execute() override;
};

///special commands
class PipeCommand : public Command {
    std::vector<std::string> leftCommand;
    std::vector<std::string> rightCommand;
    OUTPUT out;
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {};
    void execute() override;
    std::vector<std::string>& getLeft(){return this->leftCommand;};
    std::vector<std::string>& getRight(){return this->rightCommand;};
    OUTPUT getOut() {return out;}
};

class CatCommand : public BuiltInCommand {
    private:
       char* buf;
    public:
        CatCommand(const char* cmd_line): BuiltInCommand(cmd_line) {this->buf = new char[256];};
        virtual ~CatCommand() {delete this->buf;}
        void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
private:
    time_t timeForAlarm;
    time_t duration;
public:
    TimeoutCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {};
    virtual ~TimeoutCommand() {};
    void execute() override;
    time_t getTimeForAlarm() {return this->timeForAlarm;};
    time_t getDuration() {return this->duration;}
};

class SmallShell {
    private:
        pid_t pid;///maybe not necessary
        std::string lastPwd;
        std::string currentPwd;
        std::string prompt;
        JobsList jobsList;
        JobsList timeoutList;
        pid_t currCommandInFgPid;
        const char* currCommandInFgCmd;
        bool on_off;
        SmallShell();
    public:
        Command *CreateCommand(const char* cmd_line);
        SmallShell(SmallShell const&) = delete; // disable copy ctor
        void operator=(SmallShell const&) = delete; // disable = operator
        static SmallShell& getInstance() {// make SmallShell singleton
            static SmallShell instance; // Guaranteed to be destroyed.// Instantiated on first use.
            return instance;
        }
        ~SmallShell();
        void executeCommand(const char* cmd_line);
        pid_t getPid() {return pid;}
        std::string getLastPwd() {return lastPwd;}
        std::string getCurrentPwd() {return currentPwd;}
        std::string GetPrompt();
        JobsList& getJobsList();
        JobsList& getTimeoutList();
        void setPrompt(std::string newPrompt) {prompt = newPrompt;}
        void setLastPwd(std::string pwd) {lastPwd = pwd;}
        void setCurrentPwd(std::string pwd) {currentPwd = pwd;}
        void setCurrCommandInFgPid(pid_t pidCommand){ currCommandInFgPid = pidCommand;}
        pid_t getcurrCommandInFgPid(){return currCommandInFgPid;}
        void setCurrCommandInFgCmd(const char* command){ currCommandInFgCmd = command;}
        const char* getcurrCommandInFgCmd() {return this->currCommandInFgCmd;}
        void setOnOff(){this->on_off = false;}
        bool getOnOff(){return this->on_off;}
};



#endif //SMASH_COMMAND_H_
