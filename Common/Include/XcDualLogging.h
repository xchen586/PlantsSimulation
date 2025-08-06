#ifndef DUAL_LOGGING_H
#define DUAL_LOGGING_H

// Disable MSVC security warnings for standard C functions
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <memory>
#include <ctime>

namespace DualLogging {

    class DualStreamBuf : public std::streambuf {
    private:
        std::streambuf* buf1;
        std::streambuf* buf2;

    public:
        DualStreamBuf(std::streambuf* b1, std::streambuf* b2)
            : buf1(b1), buf2(b2) {
        }

    protected:
        int overflow(int c) override {
            if (c == EOF) {
                return !EOF;
            }

            int r1 = buf1->sputc(c);
            int r2 = buf2->sputc(c);

            return (r1 == EOF || r2 == EOF) ? EOF : c;
        }

        int sync() override {
            int r1 = buf1->pubsync();
            int r2 = buf2->pubsync();
            return (r1 == 0 && r2 == 0) ? 0 : -1;
        }
    };

    // Global variables to manage the dual logging state
    static std::unique_ptr<std::ofstream> g_logFile;
    static std::unique_ptr<DualStreamBuf> g_dualBuf;
    static std::streambuf* g_originalCoutBuf = nullptr;
    static std::string g_logFilePath;

    inline std::string generateLogFilePath(const char* programPath) {
        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        // Use safe time conversion based on platform
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &time_t);
#else
        tm = *std::localtime(&time_t);
#endif

        if (!programPath) {
            // Fallback to simple timestamp if no program path provided
            std::ostringstream oss;
            oss << "program_"
                << std::setfill('0') << std::setw(4) << (tm.tm_year + 1900) << "_"
                << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "_"
                << std::setfill('0') << std::setw(2) << tm.tm_mday << "_"
                << std::setfill('0') << std::setw(2) << tm.tm_hour << "_"
                << std::setfill('0') << std::setw(2) << tm.tm_min << "_"
                << std::setfill('0') << std::setw(2) << tm.tm_sec << ".log";
            return oss.str();
        }

        // Get program directory and name
        std::filesystem::path progPath(programPath);
        std::string programName = progPath.stem().string();
        std::string programDir = progPath.parent_path().string();

        // Format timestamp
        std::ostringstream oss;
        oss << programName << "_"
            << std::setfill('0') << std::setw(4) << (tm.tm_year + 1900) << "_"
            << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "_"
            << std::setfill('0') << std::setw(2) << tm.tm_mday << "_"
            << std::setfill('0') << std::setw(2) << tm.tm_hour << "_"
            << std::setfill('0') << std::setw(2) << tm.tm_min << "_"
            << std::setfill('0') << std::setw(2) << tm.tm_sec << ".log";

        // Combine directory and filename
        std::filesystem::path logPath = std::filesystem::path(programDir) / oss.str();
        return logPath.string();
    }

    inline bool setupDualLogging(const char* programPath = nullptr) {
        // If already set up, don't do it again
        if (g_originalCoutBuf != nullptr) {
            std::cout << "Dual logging already enabled. Log file: " << g_logFilePath << std::endl;
            return true;
        }

        // Generate log file path
        g_logFilePath = generateLogFilePath(programPath);

        // Open log file
        g_logFile = std::make_unique<std::ofstream>(g_logFilePath);
        if (!g_logFile->is_open()) {
            std::cerr << "Failed to open log file: " << g_logFilePath << std::endl;
            g_logFile.reset();
            return false;
        }

        // Save original cout buffer
        g_originalCoutBuf = std::cout.rdbuf();

        // Create dual stream buffer
        g_dualBuf = std::make_unique<DualStreamBuf>(g_originalCoutBuf, g_logFile->rdbuf());

        // Redirect cout to use our dual buffer
        std::cout.rdbuf(g_dualBuf.get());

        // This message will appear in both console and log file
        std::cout << "Dual logging enabled. Log file: " << g_logFilePath << std::endl;

        return true;
    }

    inline void cleanupDualLogging() {
        if (g_originalCoutBuf != nullptr) {
            // Restore original cout buffer
            std::cout.rdbuf(g_originalCoutBuf);

            // Clean up resources
            g_dualBuf.reset();
            if (g_logFile) {
                g_logFile->close();
                g_logFile.reset();
            }

            g_originalCoutBuf = nullptr;
            std::cout << "Dual logging disabled." << std::endl;
        }
    }

    inline std::string getLogFilePath() {
        return g_logFilePath;
    }

    // Automatic cleanup when program exits
    class DualLoggingCleanup {
    public:
        ~DualLoggingCleanup() {
            cleanupDualLogging();
        }
    };

    static DualLoggingCleanup g_cleanup;

} // namespace DualLogging

// Bring the main function into global namespace for convenience
using DualLogging::setupDualLogging;
using DualLogging::cleanupDualLogging;
using DualLogging::getLogFilePath;

#endif // DUAL_LOGGING_H