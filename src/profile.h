#pragma once

#include <string>
#include <sstream>
#include <cstdio>
#include "utils.h"
using namespace std;

struct Paths {
    wstring projectRoot;
    wstring executablePath;
	wstring userProfilePath;   
};

struct Profile {
    string userRootDir;
    string backgroundProcess;
    string lastRebootTime;
};


inline Paths getPaths() {
    Paths p;
    p.projectRoot = getCurrentDir();
    p.executablePath  = p.projectRoot + L"\\system.exe";
    p.userProfilePath = p.projectRoot + L"\\data\\profile.txt";
    
    if (!pathExists(p.userProfilePath)) {
        FILE* f_write = fopen(ws2s(p.userProfilePath).c_str(), "w");
        fclose(f_write);
    }

    return p;
}

inline void createProfile() {
    Paths paths = getPaths();

    FILE* f_read = fopen(ws2s(paths.userProfilePath).c_str(), "r");
    bool isBuilded = true;
    if (!f_read)
        isBuilded = false;
    else if (fgetc(f_read) == EOF)
        isBuilded = false;
    fclose(f_read);

    if (!isBuilded) {
        FILE* f_write = fopen(ws2s(paths.userProfilePath).c_str(), "w");

        fprintf(f_write, "%s=%s\n", "user_root_dir", "c:\\data\\business\\active");
        fprintf(f_write, "%s=%s\n", "background_process", "NULL");
        fprintf(f_write, "%s=%s\n", "last_reboot_time", "00:00:00");

        fclose(f_write);
    }
}

inline Profile getProfile() {
    Profile prof;

    Paths paths = getPaths();
    FILE* f = fopen(ws2s(paths.userProfilePath).c_str(), "r");
    if (!f) return prof;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), f)) {
        string line(buffer);
        stringstream ssline(line);

        string key, value;
        getline(ssline, key, '=');
        getline(ssline, value);

        if (key == "user_root_dir")
            prof.userRootDir = strip(value);
        else if (key == "background_process")
            prof.backgroundProcess = strip(value);
        else if (key == "last_reboot_time")
            prof.lastRebootTime = strip(value);
    }

    fclose(f);
    return prof;
}

inline void setProfile(const Profile& updates) {
    Profile current = getProfile();
    Profile newProf = current;

    if (!updates.userRootDir.empty())
        newProf.userRootDir = updates.userRootDir;

    if (!updates.backgroundProcess.empty())
        newProf.backgroundProcess = updates.backgroundProcess;

    if (!updates.lastRebootTime.empty())
        newProf.lastRebootTime = updates.lastRebootTime;

    Paths paths = getPaths();
    FILE* f_write = fopen(ws2s(paths.userProfilePath).c_str(), "w");

    fprintf(f_write, "%s=%s\n", "user_root_dir", newProf.userRootDir.c_str());
    fprintf(f_write, "%s=%s\n", "background_process", newProf.backgroundProcess.c_str());
    fprintf(f_write, "%s=%s\n", "last_reboot_time", newProf.lastRebootTime.c_str());

    fclose(f_write);
}
