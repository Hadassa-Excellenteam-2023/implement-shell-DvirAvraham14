#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include <string>
#include <regex>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>

#include "BackgroundProcessManager.h"

class CommandExecutor {
public:
    CommandExecutor(BackgroundProcessManager& processManager);
    void execute(const std::string& command, bool isBackground);
    enum class RedirectionType {
        INPUT,
        OUTPUT,
        CONSOLE
    };
private:
    void executeChildCommand(const std::string& command);
    //std::string replaceEnvironmentVariables(const std::string& );
    BackgroundProcessManager& backgroundProcessManager;
    RedirectionType redirectionType;
    void getRedirectionType(const std::string& command);
    void redirectInput(const std::string& command);
    void redirectOutput(const std::string& command);
};

#endif  // COMMANDEXECUTOR_H
