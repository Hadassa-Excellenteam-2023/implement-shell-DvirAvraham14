#include <iostream> // For cout, cin, etc.
#include <string> // For string
#include <vector> // For vector
#include <sstream> // For istringstream
#include <unistd.h> // For fork
#include <sys/wait.h> // For waitpid
// #include <algorithm>
#include <cstdlib> // For getenv
#include <fstream> // For file I/O
#include <regex> // For regular expressions


std::vector<pid_t> backgroundProcesses; // Track background processes

/*
 * Function to split a string by a delimiter
 * param input: The string to split
 * param delimiter: The delimiter to split by
 * return: A vector of strings
 */
std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

/*
 * Function to execute a child command
 * param command: The command to execute
 * return: None
 */
void executeChildCommand(const std::string& command) {
    std::vector<std::string> tokens = split(command, ' ');

    // Convert the vector of tokens to char* array for execvp
    std::vector<char*> args;
    for (const std::string& token : tokens) {
        if (token.front() == '$') {
            // Variable expansion
            std::string variable = token.substr(1);
            char* value = getenv(variable.c_str());
            if (value) {
                args.push_back(value);
            }
        } else {
            args.push_back(const_cast<char*>(token.c_str()));
        }
    }
    args.push_back(nullptr);

    // Execute the command
    execvp(args[0], args.data());

    // Execvp will only return if there's an error
    std::cerr << "Command not found: " << command << std::endl;
    exit(1);
}

/*
 * Function to handle the parent process
 * param pid: The PID of the child process
 * param isBackground: Whether the process is a background process
 * return: None
 */
void handleParentProcess(pid_t pid, bool isBackground) {
    if (!isBackground) {
        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);
    } else {
        // Store the background process PID
        backgroundProcesses.push_back(pid);
        std::cout << "Background process started. PID: " << pid << std::endl;
    }
}

/*
 * Function to execute a command
 * param command: The command to execute
 * param isBackground: Whether the process is a background process
 * return: None
 */
void executeCommand(const std::string& command, bool isBackground) {
    // Fork the process
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to create child process." << std::endl;
        return;
    } else if (pid == 0) {
        // Child process
        executeChildCommand(command);
    } else {
        // Parent process
        handleParentProcess(pid, isBackground);
    }
    std::ofstream historyFile("history.txt", std::ios::app);
    if (historyFile.is_open()) {
        historyFile << command << std::endl;
        historyFile.close();
    } else {
        std::cerr << "Failed to open history file." << std::endl;
    }
}

/*
 * Function to show background processes and their status (running, stopped, etc.)
 * param None
 * return: None
 */
void showJobs() {
    std::cout << "Running background processes:" << std::endl;
    for (const auto& pid : backgroundProcesses) {
        std::string status;
        int childStatus;
        pid_t result = waitpid(pid, &childStatus, WNOHANG | WUNTRACED | WCONTINUED);

        if (result == -1) {
            status = "Not found"; // Process not found or error occurred
        } else if (WIFEXITED(childStatus)) {
            status = "Exited"; // Process exited
        } else if (WIFSIGNALED(childStatus)) {
            status = "Terminated"; // Process terminated by signal
        } else if (WIFSTOPPED(childStatus)) {
            status = "Stopped"; // Process stopped by signal
        } else if (WIFCONTINUED(childStatus)) {
            status = "Running"; // Process still running
        } else {
            status = "Unknown"; // Unknown status (shouldn't happen, but just in case)
        }

        std::cout << "PID: " << pid << "   Status: " << status << std::endl;
    }
}


/*
 * Function to show the command history
 * param None
 * return: None
 */
void showHistory() {
    std::ifstream historyFile("history.txt");
    if (historyFile.is_open()) {
        std::string line;
        int count = 1;
        while (std::getline(historyFile, line)) {
            std::cout << count << ". " << line << std::endl;
            count++;
        }
        historyFile.close();
    } else {
        std::cerr << "Failed to open history file." << std::endl;
    }
}

/*
 * Function to get a command from the user
 * param None
 * return: The command entered by the user
 */
std::string getCommand() {
    std::string command;
    std::cout << "Enter a command >> ";
    std::getline(std::cin, command);
    return command;
}

/*
 * Function to check if a command is a background command
 * param command: The command to check
 * return: True if the command is a background command, false otherwise
 */
 bool isBackgroundCommand(std::string& command) {
    bool isBackground = false;
    if (!command.empty() && command.back() == '&') {
        isBackground = true;
        command.pop_back();  // Remove the '&' character
    }
    return isBackground;
}


// -------- main --------
int main() {
    while (true) {
        std::string command = getCommand();
        bool isBackground = isBackgroundCommand(command);

        if (command == "myjobs") {
            showJobs();
        } else if (command == "myhistory") {
            showHistory();
        } else {
            executeCommand(command, isBackground);
        }
    }

    return 0;
}

