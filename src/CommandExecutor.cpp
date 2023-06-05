#include "CommandExecutor.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>

CommandExecutor::CommandExecutor(BackgroundProcessManager& processManager)
        : backgroundProcessManager(processManager) {}

void CommandExecutor::execute(const std::string& command, bool isBackground) {
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to create child process." << std::endl;
        return;
    } else if (pid == 0) {
        executeChildCommand(command);
    } else {
        if (isBackground) {
            backgroundProcessManager.addBackgroundProcess(pid);
            std::cout << "Background process with PID " << pid << " started." << std::endl;
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

void CommandExecutor::executeChildCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string program;
    iss >> program;

    // Get the command arguments
    std::string argument;
    std::vector<std::string> arguments;
    while (iss >> argument) {
        arguments.push_back(argument);
    }

    // Prepare the arguments for execvp
    const char* programChar = program.c_str();
    std::vector<char*> args;
    args.push_back(const_cast<char*>(programChar));
    for (const auto& arg : arguments) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);

    // Execute the command
    if (execvp(programChar, args.data()) == -1) {
        std::cerr << "Failed to execute command: " << program << std::endl;
        exit(EXIT_FAILURE);
    }
}


