#include "CTimeCounter.h"
#include <ctime>

#if __APPLE__
int my_ctime_s(char* buffer, size_t bufferSize, const time_t* timeToConvert) {
    if (buffer == nullptr || bufferSize < 26 || timeToConvert == nullptr) {
        return 1; // Return an error code indicating invalid parameters
    }

    struct tm localTimeInfo;
    if (localtime_r(timeToConvert, &localTimeInfo) == nullptr) {
        return 1; // Error converting time
    }

    // Format the time information into the buffer
    int result = snprintf(buffer, bufferSize, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
        localTimeInfo.tm_year + 1900, localTimeInfo.tm_mon + 1, localTimeInfo.tm_mday,
        localTimeInfo.tm_hour, localTimeInfo.tm_min, localTimeInfo.tm_sec);

    if (result < 0 || result >= static_cast<int>(bufferSize)) {
        return 1; // Error in formatting or insufficient buffer size
    }

    return 0; // Success
}

void my_ctime_s_2(char* buffer, size_t bufferSize, const time_t* timeToConvert) {
    if (buffer == nullptr || bufferSize < 26 || timeToConvert == nullptr) {
        throw std::invalid_argument("Invalid parameters"); // Use C++ exception for invalid parameters
    }

    struct tm localTimeInfo;
    if (localtime_r(timeToConvert, &localTimeInfo) == nullptr) {
        throw std::runtime_error("Error converting time: " + std::string(strerror(errno)));
    }

    // Use strftime to format time
    if (strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", &localTimeInfo) == 0) {
        throw std::overflow_error("Buffer too small for formatted time");
    }
}
#endif

CTimeCounter::CTimeCounter(string& title)
	: m_title(title)
{
	start = std::chrono::system_clock::now();
	std::time_t startTime = std::chrono::system_clock::to_time_t(start);
	// Convert the time to a string using ctime_s
	char startTimeStr[26];
#if __APPLE__
    my_ctime_s(startTimeStr, sizeof(startTimeStr), &startTime);
#else
	ctime_s(startTimeStr, sizeof(startTimeStr), &startTime);
#endif
	std::cout << m_title << " Start time: " << startTimeStr << std::endl;
}

CTimeCounter::~CTimeCounter() 
{
	end = std::chrono::system_clock::now();
	std::time_t endTime = std::chrono::system_clock::to_time_t(end);
	char endTimeStr[26];
#if __APPLE__
    my_ctime_s(endTimeStr, sizeof(endTimeStr), &endTime);
#else
	ctime_s(endTimeStr, sizeof(endTimeStr), &endTime);
#endif
	std::cout << m_title << " End time: " << endTimeStr << std::endl;

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	// Convert microseconds to days, hours, minutes, seconds, and microseconds
	std::chrono::microseconds remainingTime = duration;

	long long days = std::chrono::duration_cast<std::chrono::days>(remainingTime).count();
	remainingTime -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::days(days));

	long long hours = std::chrono::duration_cast<std::chrono::hours>(remainingTime).count();
	remainingTime -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::hours(hours));

	long long minutes = std::chrono::duration_cast<std::chrono::minutes>(remainingTime).count();
	remainingTime -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::minutes(minutes));

	long long seconds = std::chrono::duration_cast<std::chrono::seconds>(remainingTime).count();
	remainingTime -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(seconds));

	long long microseconds = remainingTime.count();

	// Print the time interval in the desired format
	std::cout << m_title
		<< " Time interval: " << days << " days, "
		<< hours << " hours, "
		<< minutes << " minutes, "
		<< seconds << " seconds, "
		<< microseconds << " microseconds" << std::endl;
}


