#include "Shell.h"
#include <iostream>
#include <string>

void Shell::run() {
    while (true) {
        std::string command = getCommand();
        if (command == "exit") {
            break;
        } else if (command == "myhistory") {
            commandHistoryManager.showHistory();
        } else if (command == "myjobs") {
            backgroundProcessManager.showJobs();
        } else {
            bool isBackground = isBackgroundCommand(command);
            commandHistoryManager.addToHistory(command);
            commandExecutor.execute(command, isBackground);
        }
    }
}

std::string Shell::getCommand() {
    std::string command;
    std::cout << "shell> ";
    std::getline(std::cin, command);
    return command;
}

bool Shell::isBackgroundCommand(std::string& command) {
    size_t pos = command.find_last_not_of(" \t\n\r\f\v");
    if (pos != std::string::npos) {
        command.erase(pos + 1);
        if (command.back() == '&') {
            command.pop_back();
            return true;
        }
    }
    return false;
}