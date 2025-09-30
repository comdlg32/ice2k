#ifndef __YAPP_H
#define __YAPP_H

#include "upath.h"
#include "yarray.h"
#include "ypoll.h"
#include "ytrace.h"

class YTimer;

class YSignalPoll: public YPoll<class YApplication> {
public:
    explicit YSignalPoll(YApplication* owner) : YPoll(owner) { }
    virtual void notifyRead();
    virtual bool forRead() { return true; }
};

class IApp {
public:
    virtual ~IApp();
    virtual upath findConfigFile(upath relativePath) = 0;
    virtual void runCommand(const char *prog) = 0;
    virtual int runProgram(const char *path, const char *const *args, int fd = -1) = 0;
    virtual void exit(int exitCode) = 0;
    virtual int waitProgram(int pid) = 0;
};

class IMainLoop {
public:
    virtual ~IMainLoop();
    virtual void registerTimer(YTimer *t) = 0;
    virtual void unregisterTimer(YTimer *t) = 0;
    virtual void registerPoll(YPollBase *t) = 0;
    virtual void unregisterPoll(YPollBase *t) = 0;
    virtual void unregisterWait(YPidWaiter *w) = 0;
    virtual void registerWait(int pid, YPidWaiter* waiter) = 0;
};

class IResourceLocator {
public:
    virtual void subdirs(const char* sd, bool to, class MStringArray& ms) = 0;
};

class YApplication: public IApp, public IMainLoop, public IResourceLocator {
public:
    YApplication(int *argc, char ***argv);
    virtual ~YApplication();

    int mainLoop();
    int exitCode() const { return fExitCode; }
    void exitLoop(int exitCode);
    virtual void exit(int exitCode);

    virtual void handleSignal(int sig);
    virtual bool handleIdle();

    void catchSignal(int sig);
    void resetSignals();

    virtual int runProgram(const char *path, const char *const *args, int fd = -1);
    virtual void runCommand(const char *prog);
    virtual int waitProgram(int pid);
    virtual void registerWait(int pid, YPidWaiter* waiter);
    virtual void unregisterWait(YPidWaiter* waiter);

    virtual void subdirs(const char* sd, bool to, class MStringArray& ms);
    virtual upath findConfigFile(upath relativePath);
    static upath locateConfigFile(upath relativePath);
    static const upath& getLibDir();
    static const upath& getConfigDir();
    static const upath& getPrivConfDir(bool create = false);
    static upath getPrivConfFile(mstring basename, bool create = false);
    static upath getHomeDir();

private:
    YArray<YTimer*> timers;
    YArray<YPollBase*> polls;
    typedef YArray<YPollBase*>::IterType YPollIterType;

    struct WaitHandler {
        int pid;
        YPidWaiter* waiter;
        WaitHandler(int pid, YPidWaiter* waiter) : pid(pid), waiter(waiter) { }
    };
    YArray<WaitHandler> waits;

    YSignalPoll sfd;
    friend class YSignalPoll;

    int fLoopLevel;
    int fExitCode;
    bool fExitLoop;
    bool fExitApp;

    bool getTimeout(struct timeval *timeout);
    void handleTimeouts();
    void decreaseTimeouts(struct timeval difftime);

    void handleSignalPipe();
    void initSignals();

protected:
    friend class YTimer;
    friend class YPollBase;

    virtual void registerTimer(YTimer *t);
    virtual void unregisterTimer(YTimer *t);
    bool nextTimeout(struct timeval *timeout);
    bool nextTimeoutWithFuzziness(struct timeval *timeout);
    virtual void registerPoll(YPollBase *t);
    virtual void unregisterPoll(YPollBase *t);

protected:
    virtual void flushXEvents() {}
    virtual bool handleXEvents() { return false; }

    void closeFiles();
};

extern IMainLoop *mainLoop;


#endif

// vim: set sw=4 ts=4 et:
