#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "utils.h"

inline void rebootPC() {
	system("shutdown /r /t 0");
}

/*
struct Paths {
    wstring projectRoot;
    wstring executablePath;
	wstring userProfilePath;   
    wstring executablesList;
};

struct Profile {
    string userRootDir;
    string backgroundProcess;
    string lastRebootTime;
};
*/
