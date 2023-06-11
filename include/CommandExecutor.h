#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include <string>
#include <regex>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <array>


#include "BackgroundProcessManager.h"

enum class RedirectionType {
    CONSOLE,
    INPUT,
    OUTPUT
};

class CommandExecutor {
public:
    explicit CommandExecutor(BackgroundProcessManager& processManager);

    void execute(const std::string& command, bool isBackground);

private:
    BackgroundProcessManager& backgroundProcessManager;
    RedirectionType redirectionType;

    void executePipeline(const std::vector<std::string>& commands, bool isBackground);
    void executeSingleCommand(const std::string& command, bool isBackground);
    std::vector<std::string> parsePipelineCommands(const std::string& command);
    void executeChildCommand(const std::string& command, int inputFD, int outputFD);
    void getRedirectionType(const std::string& command);
    void redirectInput(const std::string& command, int inputFD);
    void redirectOutput(const std::string& command, int outputFD);
};

#endif  // COMMANDEXECUTOR_H
