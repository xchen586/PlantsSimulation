#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

class PsIniParser {
public:
    bool Parse(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }

        std::string currentSection;
        std::string line;
        while (std::getline(file, line)) {
            ParseLine(line, currentSection);
        }

        file.close();
        return true;
    }

    std::string GetValue(const std::string& section, const std::string& key) const {
        auto sectionIter = data.find(section);
        if (sectionIter != data.end()) {
            auto keyIter = sectionIter->second.find(key);
            if (keyIter != sectionIter->second.end()) {
                return keyIter->second;
            }
        }
        return ""; // Return empty string if section or key not found
    }

private:
    void ParseLine(const std::string& line, std::string& currentSection) {
        std::istringstream iss(line);
        std::string token;

        // Check if the line is a comment
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            // Remove the comment part
            token = line.substr(0, commentPos);
        }
        else {
            token = line;
        }

        commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            // Remove the comment part
            token = line.substr(0, commentPos);
        }
        else {
            token = line;
        }

        // Trim leading and trailing whitespaces
        size_t firstChar = token.find_first_not_of(" \t\r\n");
        size_t lastChar = token.find_last_not_of(" \t\r\n");
        if (firstChar != std::string::npos && lastChar != std::string::npos) {
            token = token.substr(firstChar, lastChar - firstChar + 1);
        }

        if (!token.empty()) {
            if (token[0] == '[' && token.back() == ']') {
                // It's a section header
                currentSection = token.substr(1, token.size() - 2);
            }
            else {
                // It's a key-value pair
                size_t equalPos = token.find('=');
                if (equalPos != std::string::npos) {
                    std::string key = token.substr(0, equalPos);
                    std::string value = token.substr(equalPos + 1);
                    // Trim leading and trailing whitespaces from key and value
                    key = TrimWhitespace(key);
                    value = TrimWhitespace(value);
                    if (!currentSection.empty()) {
                        data[currentSection][key] = value;
                    }
                }
            }
        }
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data;

    static std::string TrimWhitespace(const std::string& str) {
        size_t firstChar = str.find_first_not_of(" \t\r\n");
        size_t lastChar = str.find_last_not_of(" \t\r\n");
        if (firstChar != std::string::npos && lastChar != std::string::npos) {
            return str.substr(firstChar, lastChar - firstChar + 1);
        }
        return "";
    }
};

const char* GetIniValue(const PsIniParser& parser, const string& section, const string& key) {
    std::string value = parser.GetValue(std::string(section), std::string(key));
    // Make sure to allocate a new char array and copy the string content
    // This memory should be managed appropriately based on your use case
    // Allocate memory for the result
    char* result = new char[value.length() + 1];
#if __APPLE__
    strlcpy(result, value.c_str(), value.length() + 1);
#else
    // Use strcpy_s for safer string copying
    strcpy_s(result, value.length() + 1, value.c_str());
#endif
    return result;
}