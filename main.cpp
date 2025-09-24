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
	
	logger.Log("#==========#");
	logger.Log("Background service booted at -> TIME: [" + getTime_now() + "] and DAY: [" + getDay_today() + "]");
	
	vector<string> exes;
	string search = "main.py";
	getExecutables_fromDir(search, profile.userRootDir, exes);
	
	logger.Log("Executables got it from [" + profile.userRootDir + "] to search [" + search + "]");
	for (auto& exe : exes) {
		exe = "python " + exe;
		logger.Log(" " + exe + "");
	}
	
	vector<DWORD> pids;
	ProcessManager manager;
	for (auto& exe : exes) pids.push_back(manager.StartProcess(exe));
	
	string priority = "BELOW_NORMAL_PRIORITY_CLASS";
	while (true) {
		if (profile.lastRebootTime == getTime_now()) {
			logger.Log("About to close processes");
			manager.TerminateAllProcesses();
			
			logger.Log("About to reboot pc at time: [" + getTime_now() + "]");
			logger.ClearLog();
			rebootPC();
		}
		
		if (isUserActive(300)) {
			logger.Log("User on pc");
			
			if (priority != "BELOW_NORMAL_PRIORITY_CLASS") {
				for (auto& pid : pids) manager.SetProcessPriority(pid, BELOW_NORMAL_PRIORITY_CLASS); 
				priority = "BELOW_NORMAL_PRIORITY_CLASS";
			}
		} else {
			logger.Log("User off pc");
			
			if (priority != "HIGH_PRIORITY_CLASS") {
				for (auto& pid : pids) manager.SetProcessPriority(pid, HIGH_PRIORITY_CLASS);
				priority = "HIGH_PRIORITY_CLASS";	
			}
		}
		
		vector<DWORD> aliveProcesses = manager.GetActiveProcesses();
		
		logger.Log("Processes setted at priority -> " + priority);
		logger.Log("Alive process: ");
		for (auto& pid : aliveProcesses) logger.Log(" -> " + to_string(pid));
		
		sleepfor(5);
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
