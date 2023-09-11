#include "CTimeCounter.h"

CTimeCounter::CTimeCounter(string& title)
	: m_title(title)
{
	start = std::chrono::system_clock::now();
	std::time_t startTime = std::chrono::system_clock::to_time_t(start);
	// Convert the time to a string using ctime_s
	char startTimeStr[26];
	ctime_s(startTimeStr, sizeof(startTimeStr), &startTime);
	std::cout << title << " Start time: " << startTimeStr << std::endl;
}

CTimeCounter::~CTimeCounter() 
{
	end = std::chrono::system_clock::now();
	std::time_t endTime = std::chrono::system_clock::to_time_t(end);
	char endTimeStr[26];
	ctime_s(endTimeStr, sizeof(endTimeStr), &endTime);
	std::cout << "End time: " << endTimeStr << std::endl;

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
	std::cout << "Time interval: " << days << " days, "
		<< hours << " hours, "
		<< minutes << " minutes, "
		<< seconds << " seconds, "
		<< microseconds << " microseconds" << std::endl;
}