#ifndef SHELL_H
#define SHELL_H

#include "CommandExecutor.h"
#include "BackgroundProcessManager.h"
#include "CommandHistoryManager.h"

class Shell {
public:
    void run();

private:
    BackgroundProcessManager backgroundProcessManager;
    CommandExecutor commandExecutor {backgroundProcessManager};
    CommandHistoryManager commandHistoryManager;
    std::string getCommand();
    bool isBackgroundCommand(std::string& command);
};

#endif  // SHELL_H
