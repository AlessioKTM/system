#pragma once

#include <time.h>
#include <vector>
#include <string>
#include <sstream>
#include <windows.h>
using namespace std;

void getExecutables_fromDir(const string& searchName, const string& dir, vector<string>& results) {
    string searchPath = dir + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    do {
        string name = findData.cFileName;

        if (name != "." && name != "..") {
            string fullPath = dir + "\\" + name;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            	getExecutables_fromDir(searchName, fullPath, results);
            else {
                if (name == searchName) results.push_back(fullPath);
            }
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
}

bool isUserActive(DWORD idleThresholdSec = 60) {
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);
    lii.dwTime = 0;

    if (GetLastInputInfo(&lii)) {
        DWORD currentTick = GetTickCount();
        DWORD idleSec = (currentTick - lii.dwTime) / 1000;
        return idleSec < idleThresholdSec;
    }

    return false;
}

inline void sleepfor(int ntime) {
	int secs = ntime * 1000;
	Sleep(secs);
}

inline string ws2s(const wstring& wstr) {
    if (wstr.empty()) return string();
    int sizeNeeded = WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(),
        (int)wstr.size(), nullptr, 0,
        nullptr, nullptr
	);
	
    string result(sizeNeeded, 0);
    WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(), (int)wstr.size(),
        &result[0], sizeNeeded, nullptr, nullptr
	);
	
    return result;
}

inline wstring s2ws(const string& str) {
	if (str.empty()) return wstring();
    int sizeNeeded = MultiByteToWideChar(
        CP_UTF8, 0, str.c_str(),
        (int)str.size(), nullptr, 0
    );
    
    wstring result(sizeNeeded, 0);
    MultiByteToWideChar(
        CP_UTF8, 0, str.c_str(), (int)str.size(),
        &result[0], sizeNeeded
    );
    
}

inline string strip(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

inline bool pathExists(const wstring& path) {
	DWORD attr = GetFileAttributesW(path.c_str());
	if (attr != INVALID_FILE_ATTRIBUTES)
		return true;
	return false;
}

inline wstring getCurrentDir() {
	wchar_t buffer[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, buffer, MAX_PATH);
	
	wstring fullPath(buffer);
	size_t pos = fullPath.find_last_of(L"\\/");
	
	return fullPath.substr(0, pos);
}

inline string getDay_today() {
	long long int today = time(nullptr);
	string today_string = ctime(&today);
	
	stringstream ss(today_string);
	string dayOfWeek, month, day, tmp, year;
	
	getline(ss, dayOfWeek, ' ');
	getline(ss, month, ' ');
	getline(ss, day, ' ');
	getline(ss, tmp, ' ');
	getline(ss, year, ' ');
	
	string result = day + "." + month + "." + year;
	result = strip(result);
	return result;
}

inline string getTime_now() {
	long long int now = time(nullptr);
	string now_string = ctime(&now);
	
	stringstream ss(now_string);
	string result = "";
	
	for (int i = 0; i < 4; i++) getline(ss, result, ' ');
	
	return result;
}
