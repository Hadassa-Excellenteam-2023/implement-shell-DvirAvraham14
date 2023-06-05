#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include <string>
#include <regex>
#include <cstdlib>
#include <unistd.h>

#include "BackgroundProcessManager.h"

class CommandExecutor {
public:
    CommandExecutor(BackgroundProcessManager& processManager);
    void execute(const std::string& command, bool isBackground);

private:
    void executeChildCommand(const std::string& command);
    std::string replaceEnvironmentVariables(const std::string& );
    BackgroundProcessManager& backgroundProcessManager;
};

#endif  // COMMANDEXECUTOR_H
