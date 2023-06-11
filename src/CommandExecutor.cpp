#include "CommandExecutor.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

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
    getRedirectionType(command);

    // Get the command arguments
    std::string argument;
    std::vector<std::string> arguments;
    while (iss >> argument) {
        if (argument == "<" || argument == ">") {
            // Skip redirection operator and filename
            iss >> argument;
        } else {
            arguments.push_back(argument);
        }
    }

    // Prepare the arguments for execvp
    const char* programChar = program.c_str();
    std::vector<char*> args;
    args.push_back(const_cast<char*>(programChar));
    for (const auto& arg : arguments) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);

    // Redirect input/output
    if (redirectionType == RedirectionType::INPUT) {
        redirectInput(command);
    } else if (redirectionType == RedirectionType::OUTPUT) {
        redirectOutput(command);
    }

    // Execute the command
    if (execvp(programChar, args.data()) == -1) {
        std::cerr << "Failed to execute command: " << program << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CommandExecutor::getRedirectionType(const std::string& command) {
    if (command.find("<") != std::string::npos) {
        this->redirectionType = RedirectionType::INPUT;
    } else if (command.find(">") != std::string::npos) {
        this->redirectionType = RedirectionType::OUTPUT;
    } else {
        this->redirectionType = RedirectionType::CONSOLE;
    }
}

void CommandExecutor::redirectInput(const std::string& command) {
    std::istringstream iss(command);
    std::string inputFile;
    std::string argument;
    while (iss >> argument) {
        if (argument == "<") {
            iss >> inputFile;
            break;
        }
    }
    if (!inputFile.empty()) {
        std::ifstream input(inputFile);
        if (!input) {
            std::cerr << "Failed to open input file: " << inputFile << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string fileContent((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        input.close();

        std::string modifiedCommand = command;
        size_t pos = modifiedCommand.find("<");
        if (pos != std::string::npos) {
            modifiedCommand.replace(pos, inputFile.length() + 1, fileContent);
        }

        executeChildCommand(modifiedCommand);
    }
}


void CommandExecutor::redirectOutput(const std::string& command) {
    std::istringstream iss(command);
    std::string outputFile;
    std::string argument;
    while (iss >> argument) {
        if (argument == ">") {
            iss >> outputFile;
            break;
        }
    }
    if (!outputFile.empty()) {
        int outputFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outputFd == -1) {
            std::cerr << "Failed to open output file: " << outputFile << std::endl;
            exit(EXIT_FAILURE);
        }
        if (dup2(outputFd, STDOUT_FILENO) == -1) {
            std::cerr << "Failed to redirect output file descriptor." << std::endl;
            exit(EXIT_FAILURE);
        }
        close(outputFd);
    }
}

