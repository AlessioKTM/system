#include <iostream>
#include <string>
#include <windows.h>

#include "src\profile.h"
#include "src\utils.h"
#include "src\background.h"
#include "src\sys.h"
#include "src\log.h"
using namespace std;

void runBackgroundService() {
	auto profile = getProfile();
	auto path = getPaths();
	
	Logger logger; 
	logger.SetFileLog("c:\\data\\system\\data");
	
	logger.Log("#==========#\n");
	logger.Log("Background service booted at time: [" + getTime_now() + "]\n");
	
	vector<string> exes;
	string search = "main.py";
	getExecutables_fromDir(search, profile.userRootDir, exes);
	
	logger.Log("Executables got it from {" + profile.userRootDir + "} to search {" + search + "}\n");
	for (auto& exe : exes) {
		exe = "python " + exe;
		logger.Log(" " + exe + "\n");
	}
	
	vector<DWORD> pids;
	ProcessManager manager;
	for (auto& exe : exes) pids.push_back(manager.StartProcess(exe));
	
	string priority = "BELOW_NORMAL_PRIORITY_CLASS";
	while (true) {
		if (profile.lastRebootTime == getTime_now()) {
			logger.Log("About to close processes\n");
			manager.TerminateAllProcesses();
			
			logger.Log("About to reboot pc at time: [" + getTime_now() + "]\n");
			rebootPC();
		}
		
		if (isUserActive(300)) {
			logger.Log("User online\n");
			
			if (priority == "BELOW_NORMAL_PRIORITY_CLASS") {
				for (auto& pid : pids) manager.SetProcessPriority(pid, HIGH_PRIORITY_CLASS); 
				priority = "HIGH_PRIORITY_CLASS";
			}
		} else {
			logger.Log("User offline\n");
			
			if (priority == "HIGH_PRIORITY_CLASS") {
				for (auto& pid : pids) manager.SetProcessPriority(pid, BELOW_NORMAL_PRIORITY_CLASS);		
				priority = "BELOW_NORMAL_PRIORITY_CLASS";
			}
		}
		logger.Log("\n Processes setted at priority -> " + priority + "\n");
		sleepfor(1);
	}
}

string runUserService() {
	cout<< "This is run user service.";
	return "END";
}

/* ---------- ---------- ---------- */

int main(int argc, char* argv[]) {
	createProfile();
	
	bool isBackground = (argc > 1 && string(argv[1]) == "--bg");
	
	if (isBackground) {
		HWND hWindow = GetConsoleWindow();
		ShowWindow(hWindow, SW_HIDE);	
		runBackgroundService();
	} else {
		string result = runUserService();
		if (result == "END") cout<< " END PROGRAM";
	}
	
	return 0;
}
