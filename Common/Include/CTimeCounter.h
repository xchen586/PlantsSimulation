#pragma once
#include <string>
#include <chrono>
#include <iostream>

using namespace std;

class CTimeCounter
{
public:
	CTimeCounter(string& title);
	~CTimeCounter();
private:
	string m_title;
	chrono::system_clock::time_point start;
	chrono::system_clock::time_point end;
};

