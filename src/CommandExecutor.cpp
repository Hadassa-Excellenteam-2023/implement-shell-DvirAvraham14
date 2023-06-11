#include "CommandExecutor.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Trim leading and trailing whitespaces from a string
void trim(std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos)
        str = str.substr(start, end - start + 1);
    else
        str.clear();
}


/*
 * Constructor
 */
CommandExecutor::CommandExecutor(BackgroundProcessManager& processManager)
        : backgroundProcessManager(processManager) {}


/*
 * Execute the command - either a single command or a pipeline of commands
 * @param command The command to execute
 * @param isBackground Whether the command is a background command
 * @return void
 */
void CommandExecutor::execute(const std::string& command, bool isBackground) {
    if (command.find('|') != std::string::npos) {
        std::vector<std::string> commands = parsePipelineCommands(command);
        executePipeline(commands, isBackground);
    } else {
        executeSingleCommand(command, isBackground);
    }
}

/*
 * Execute a pipeline of commands
 * @param commands The commands in the pipeline
 * @param isBackground Whether the pipeline is a background command
 * @return void
 */
void CommandExecutor::executePipeline(const std::vector<std::string>& commands, bool isBackground) {
    int pipeCount = commands.size() - 1;
    std::vector<int> pipefds(2 * pipeCount);

    // Create pipes for each command in the pipeline
    for (int i = 0; i < pipeCount; ++i) {
        if (pipe(&pipefds[2 * i]) < 0) {
            std::cerr << "Failed to create pipe." << std::endl;
            return;
        }
    }

    // Execute commands in the pipeline
    for (int i = 0; i < commands.size(); ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Failed to create child process." << std::endl;
            return;
        } else if (pid == 0) {
            // Child process
            if (i == 0) {
                // First command, redirect output to pipe
                dup2(pipefds[1], STDOUT_FILENO);
            } else if (i == commands.size() - 1) {
                // Last command, redirect input from pipe
                dup2(pipefds[2 * (i - 1)], STDIN_FILENO);
            } else {
                // Middle commands, redirect input from previous pipe and output to next pipe
                dup2(pipefds[2 * (i - 1)], STDIN_FILENO);
                dup2(pipefds[2 * i + 1], STDOUT_FILENO);
            }

            // Close all pipe file descriptors in the child process
            for (int j = 0; j < 2 * pipeCount; ++j) {
                close(pipefds[j]);
            }

            executeSingleCommand(commands[i], false);
            exit(0);
        }
    }

    // Close all pipe file descriptors in the parent process
    for (int i = 0; i < 2 * pipeCount; ++i) {
        close(pipefds[i]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < commands.size(); ++i) {
        int status;
        wait(&status);
    }
}


/*
 * parsePipelineCommands - Parse a pipeline of commands into a vector of commands
 * @param command The pipeline of commands
 * @return A vector of commands
 */
std::vector<std::string> CommandExecutor::parsePipelineCommands(const std::string& command) {
    std::vector<std::string> commands;
    std::istringstream iss(command);
    std::string token;
    while (getline(iss, token, '|')) {
        commands.push_back(token);
    }
    return commands;
}


/*
 * executeSingleCommand - Execute a single command
 * @param command The command to execute
 * @param isBackground Whether the command is a background command
 */
void CommandExecutor::executeSingleCommand(const std::string& command, bool isBackground) {
    int inputFD = STDIN_FILENO;
    int outputFD = STDOUT_FILENO;

    if (redirectionType == RedirectionType::INPUT) {
        redirectInput(command, inputFD);
    } else if (redirectionType == RedirectionType::OUTPUT) {
        redirectOutput(command, outputFD);
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to create child process." << std::endl;
        return;
    } else if (pid == 0) {
        executeChildCommand(command, inputFD, outputFD);
        exit(0);
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


/*
 * executeChildCommand - Execute a child command
 * @param command The command to execute
 * @param inputFD The file descriptor for input redirection
 * @param outputFD The file descriptor for output redirection
 * @return void
 * @note This function is only called in the child process
 */
void CommandExecutor::executeChildCommand(const std::string& command, int inputFD, int outputFD) {
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
        redirectInput(command, inputFD);
    } else if (redirectionType == RedirectionType::OUTPUT) {
        redirectOutput(command, outputFD);
    }

    // Execute the command
    if (execvp(programChar, args.data()) == -1) {
        std::cerr << "Failed to execute command: " << program << std::endl;
        exit(EXIT_FAILURE);
    }
}


/*
 * getRedirectionType - Get the redirection type of a command
 * @param command The command to get redirection type
 * @return void
 */
void CommandExecutor::getRedirectionType(const std::string& command) {
    if (command.find(">") != std::string::npos) {
        redirectionType = RedirectionType::OUTPUT;
    } else if (command.find("<") != std::string::npos) {
        redirectionType = RedirectionType::INPUT;
    } else {
        redirectionType = RedirectionType::CONSOLE;
    }
}


/*
 * redirectInput - Redirect input from a file
 * @param command The command to redirect input
 * @param inputFD The file descriptor for input redirection
 * @return void
 */
void CommandExecutor::redirectInput(const std::string& command, int inputFD) {
    size_t found = command.find("<");
    std::string inputFile = command.substr(found + 1);
    trim(inputFile);
    inputFD = open(inputFile.c_str(), O_RDONLY);
    if (inputFD == -1) {
        std::cerr << "Failed to open input file: " << inputFile << std::endl;
        exit(EXIT_FAILURE);
    }
    if (dup2(inputFD, STDIN_FILENO) == -1) {
        std::cerr << "Failed to redirect input." << std::endl;
        exit(EXIT_FAILURE);
    }
    close(inputFD);
}


/*
 * redirectOutput - Redirect output to a file
 * @param command The command to redirect output
 * @param outputFD The file descriptor for output redirection
 * @return void
 */
void CommandExecutor::redirectOutput(const std::string& command, int outputFD) {
    size_t found = command.find(">");
    std::string outputFile = command.substr(found + 1);
    trim(outputFile);
    outputFD = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (outputFD == -1) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        exit(EXIT_FAILURE);
    }
    if (dup2(outputFD, STDOUT_FILENO) == -1) {
        std::cerr << "Failed to redirect output." << std::endl;
        exit(EXIT_FAILURE);
    }
    close(outputFD);
}
