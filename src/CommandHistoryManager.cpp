#include "CommandHistoryManager.h"
#include <iostream>
#include <fstream>

void CommandHistoryManager::addToHistory(const std::string& command) {
    commandHistory.push_back(command);
}

void CommandHistoryManager::showHistory() {
    int count = 1;
    for (const auto& line : commandHistory) {
        std::cout << count << ". " << line << std::endl;
        count++;
    }
}
