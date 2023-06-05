#ifndef BACKGROUNDSPROCESSMANAGER_H
#define BACKGROUNDSPROCESSMANAGER_H

#include <vector>
#include <sys/wait.h>

class BackgroundProcessManager {
public:
    void addBackgroundProcess(pid_t pid);
    void showJobs();

private:
    std::vector<pid_t> backgroundProcesses;
};

#endif  // BACKGROUNDSPROCESSMANAGER_H
