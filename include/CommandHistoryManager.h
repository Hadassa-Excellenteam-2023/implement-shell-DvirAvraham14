#ifndef COMMANDHISTORYMANAGER_H
#define COMMANDHISTORYMANAGER_H

#include <string>
#include <vector>

class CommandHistoryManager {
public:
    void addToHistory(const std::string& command);
    void showHistory();

private:
    std::vector<std::string> commandHistory;
};

#endif  // COMMANDHISTORYMANAGER_H
