#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>


std::string getEnvValue(const std::string& key) {
    std::ifstream file("data.env"); // Assuming the env file is named ".env" and is in the same directory
    if (!file.is_open()) {
        std::cerr << "Error: Could not open .env file." << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;

        std::string fileKey = line.substr(0, delimiterPos);
        std::string fileValue = line.substr(delimiterPos + 1);

        // Trim whitespace (optional, improves robustness)
        fileKey.erase(0, fileKey.find_first_not_of(" \t"));
        fileKey.erase(fileKey.find_last_not_of(" \t") + 1);
        fileValue.erase(0, fileValue.find_first_not_of(" \t"));
        fileValue.erase(fileValue.find_last_not_of(" \t") + 1);

        if (fileKey == key) {
            return fileValue;
        }
    }

    return ""; // Key not found
}

int main(){
    std::cout << getEnvValue("API_KEY") << std::endl;
    std::cout << getEnvValue("IP_ADDR") << std::endl;
    return 0;
}