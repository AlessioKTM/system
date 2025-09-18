#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <memory>
using namespace std;

struct ProcessInfo {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD processId;
    string executablePath;
    bool isValid;
    DWORD priorityClass;
    DWORD_PTR cpuAffinity;
    
    ProcessInfo() : hProcess(NULL), hThread(NULL), processId(0), isValid(false), 
                    priorityClass(NORMAL_PRIORITY_CLASS), cpuAffinity(0) {}
    
    ~ProcessInfo() {
        if (hProcess != NULL) CloseHandle(hProcess);
        if (hThread != NULL) CloseHandle(hThread);
    }
};

class ProcessManager {
	private:
	    map<DWORD, shared_ptr<ProcessInfo>> processes;
	    
	    // Funzione per risolvere il comando Python
	    string resolvePythonCommand(const string& command) {
	        // Controlla se il comando inizia con "python"
	        if (command.length() >= 6 && command.substr(0, 6) == "python") {
	            // Trova dove finisce il comando python e iniziano gli argomenti
	            size_t spacePos = command.find(' ');
	            if (spacePos != string::npos) {
	                string pythonCmd = command.substr(0, spacePos);
	                string scriptAndArgs = command.substr(spacePos + 1);
	                
	                // Risolvi il percorso completo di python.exe
	                string pythonPath = findPythonExecutable(pythonCmd);
	                if (!pythonPath.empty()) {
	                    return pythonPath + " " + scriptAndArgs;
	                }
	            }
	        }
	        return command;
	    }
	    
	    // Trova l'eseguibile Python nel PATH
	    string findPythonExecutable(const string& pythonCmd) {
	        vector<string> extensions = {".exe", ".cmd", ".bat"};
	        vector<string> pythonNames = {pythonCmd, "python", "python3"};
	        
	        // Prima prova con il comando così com'è
	        for (const string& ext : extensions) {
	            string fullCmd = pythonCmd + ext;
	            string path = searchInPath(fullCmd);
	            if (!path.empty()) return path;
	        }
	        
	        // Se non trova, prova con python e python3
	        if (pythonCmd != "python" && pythonCmd != "python3") {
	            for (const string& name : pythonNames) {
	                for (const string& ext : extensions) {
	                    string fullCmd = name + ext;
	                    string path = searchInPath(fullCmd);
	                    if (!path.empty()) return path;
	                }
	            }
	        }
	        
	        return "";
	    }
	    
	    // Cerca un eseguibile nel PATH
	    string searchInPath(const string& executable) {
	        const char* pathEnv = getenv("PATH");
	        if (!pathEnv) return "";
	        
	        string pathStr(pathEnv);
	        size_t start = 0;
	        size_t end = 0;
	        
	        while ((end = pathStr.find(';', start)) != string::npos) {
	            string path = pathStr.substr(start, end - start);
	            if (!path.empty()) {
	                if (path.back() != '\\') path += "\\";
	                string fullPath = path + executable;
	                
	                // Controlla se il file esiste usando CreateFile
	                HANDLE hFile = CreateFileA(fullPath.c_str(), 0, FILE_SHARE_READ,
	                                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	                if (hFile != INVALID_HANDLE_VALUE) {
	                    CloseHandle(hFile);
	                    return fullPath;
	                }
	            }
	            start = end + 1;
	        }
	        
	        // Controlla l'ultimo percorso
	        if (start < pathStr.length()) {
	            string path = pathStr.substr(start);
	            if (!path.empty()) {
	                if (path.back() != '\\') path += "\\";
	                string fullPath = path + executable;
	                
	                HANDLE hFile = CreateFileA(fullPath.c_str(), 0, FILE_SHARE_READ,
	                                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	                if (hFile != INVALID_HANDLE_VALUE) {
	                    CloseHandle(hFile);
	                    return fullPath;
	                }
	            }
	        }
	        
	        return "";
	    }
	    
	    // Estrae il percorso della directory di lavoro dal comando
	    string getWorkingDirectory(const string& command) {
	        // Se il comando contiene python, cerca lo script .py
	        if (command.find("python") != string::npos) {
	            size_t pyPos = command.find(".py");
	            if (pyPos != string::npos) {
	                // Trova l'inizio del percorso del file .py
	                size_t start = command.rfind(' ', pyPos);
	                if (start == string::npos) start = 0;
	                else start++;
	                
	                string scriptPath = command.substr(start, pyPos + 3 - start);
	                return getExecutableFolder(scriptPath);
	            }
	        }
	        
	        // Altrimenti usa il metodo originale
	        size_t spacePos = command.find(' ');
	        string exePath = (spacePos != string::npos) ? command.substr(0, spacePos) : command;
	        return getExecutableFolder(exePath);
	    }
	// END private
	    
	public:
		string getExecutableFolder(const string& exePath) {
			size_t pos = exePath.find_last_of("\\/");
			if (pos != string::npos) return exePath.substr(0, pos);
			return ".";
		}
		
	    DWORD StartProcess(const string& executablePath, const string& arguments = "", 
	                      bool lowPriority = false, int limitCpuCores = 0) {
	        STARTUPINFOA si;
	        PROCESS_INFORMATION pi;
	        
	        ZeroMemory(&si, sizeof(si));
	        ZeroMemory(&pi, sizeof(pi));
	        si.cb = sizeof(si);
	        si.dwFlags = STARTF_USESHOWWINDOW;
	        si.wShowWindow = SW_HIDE;
	        
	        // Risolvi il comando se è un comando Python
	        string resolvedPath = resolvePythonCommand(executablePath);
	        
	        string commandLine = resolvedPath;
	        if (!arguments.empty()) {
	            commandLine += " " + arguments;
	        }
	        
	        vector<char> cmdLine(commandLine.begin(), commandLine.end());
	        cmdLine.push_back('\0');
	        
	        DWORD creationFlags = CREATE_NO_WINDOW;
	        if (lowPriority) {
	            creationFlags |= BELOW_NORMAL_PRIORITY_CLASS;
	        }
	        
	        string workingDir = getWorkingDirectory(resolvedPath);
	        BOOL success = CreateProcessA(
	            NULL,
	            cmdLine.data(),
	            NULL, NULL, FALSE, creationFlags, NULL, workingDir.c_str(),
	            &si, &pi
	        );
	        
	        if (success) {
	            auto procInfo = make_shared<ProcessInfo>();
	            procInfo->hProcess = pi.hProcess;
	            procInfo->hThread = pi.hThread;
	            procInfo->processId = pi.dwProcessId;
	            procInfo->executablePath = executablePath; // Mantieni il comando originale
	            procInfo->isValid = true;
	            procInfo->priorityClass = lowPriority ? BELOW_NORMAL_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
	            
	            if (limitCpuCores > 0) {
	                SetProcessCpuAffinity(pi.dwProcessId, limitCpuCores);
	            }
	            
	            processes[pi.dwProcessId] = procInfo;
	            
	            cout << "Process started: " << executablePath 
	                 << " (PID: " << pi.dwProcessId;
	            if (lowPriority) cout << ", Priority: LOW";
	            if (limitCpuCores > 0) cout << ", CPU limited to " << limitCpuCores << " cores";
	            cout << ")" << endl;
	            
	            return pi.dwProcessId;
	        } else {
	            DWORD error = GetLastError();
	            cout << "Error starting process " << executablePath 
	                 << ". Error code: " << error << endl;
	            return 0;
	        }
	    }
	    
	    bool TerminateProcess(DWORD processId, bool forceKill = false) {
	        auto it = processes.find(processId);
	        if (it == processes.end()) {
	            cout << "Process PID " << processId << " not found." << endl;
	            return false;
	        }
	        
	        auto procInfo = it->second;
	        bool success = TerminateProcessInternal(procInfo, forceKill);
	        
	        if (success) {
	            processes.erase(it);
	        }
	        
	        return success;
	    }
	    
	    int TerminateAllProcesses(bool forceKill = false) {
	        int terminated = 0;
	        auto it = processes.begin();
	        
	        while (it != processes.end()) {
	            if (TerminateProcessInternal(it->second, forceKill)) {
	                terminated++;
	            }
	            it = processes.erase(it);
	        }
	        
	        cout << "Terminated " << terminated << " processes." << endl;
	        return terminated;
	    }
	    
	    bool IsProcessRunning(DWORD processId) {
	        auto it = processes.find(processId);
	        if (it == processes.end()) {
	            return false;
	        }
	        
	        DWORD exitCode;
	        if (GetExitCodeProcess(it->second->hProcess, &exitCode)) {
	            if (exitCode != STILL_ACTIVE) {
	                processes.erase(it);
	                return false;
	            }
	            return true;
	        }
	        
	        return false;
	    }
	    
	    vector<DWORD> GetActiveProcesses() {
	        vector<DWORD> activeProcesses;
	        auto it = processes.begin();
	        
	        while (it != processes.end()) {
	            if (IsProcessRunning(it->first)) {
	                activeProcesses.push_back(it->first);
	                ++it;
	            } else {
	                it = processes.erase(it);
	            }
	        }
	        
	        return activeProcesses;
	    }
	    
	    void ShowProcessStatus() {
	        cout << "\n=== PROCESS STATUS ===" << endl;
	        auto activeProcs = GetActiveProcesses();
	        
	        if (activeProcs.empty()) {
	            cout << "No active processes." << endl;
	        } else {
	            for (DWORD pid : activeProcs) {
	                auto it = processes.find(pid);
	                if (it != processes.end()) {
	                    cout << "PID " << pid << ": " << it->second->executablePath;
	                    
	                    switch(it->second->priorityClass) {
	                        case IDLE_PRIORITY_CLASS: cout << " [IDLE]"; break;
	                        case BELOW_NORMAL_PRIORITY_CLASS: cout << " [LOW]"; break;
	                        case NORMAL_PRIORITY_CLASS: cout << " [NORMAL]"; break;
	                        case ABOVE_NORMAL_PRIORITY_CLASS: cout << " [HIGH]"; break;
	                        case HIGH_PRIORITY_CLASS: cout << " [VERY_HIGH]"; break;
	                    }
	                    
	                    if (it->second->cpuAffinity != 0) {
	                        int cores = __builtin_popcountll(it->second->cpuAffinity);
	                        cout << " [" << cores << " CPU cores]";
	                    }
	                    
	                    cout << endl;
	                }
	            }
	        }
	        cout << "===================\n" << endl;
	    }
	    
	    bool SetProcessCpuAffinity(DWORD processId, int maxCores) {
	        auto it = processes.find(processId);
	        if (it == processes.end()) return false;
	        
	        SYSTEM_INFO sysInfo;
	        GetSystemInfo(&sysInfo);
	        int totalCores = sysInfo.dwNumberOfProcessors;
	        
	        if (maxCores <= 0 || maxCores > totalCores) {
	            maxCores = totalCores;
	        }
	        
	        DWORD_PTR affinityMask = 0;
	        for (int i = 0; i < maxCores; i++) {
	            affinityMask |= (1ULL << i);
	        }
	        
	        bool success = SetProcessAffinityMask(it->second->hProcess, affinityMask);
	        if (success) {
	            it->second->cpuAffinity = affinityMask;
	            cout << "CPU affinity set for PID " << processId 
	                 << " (" << maxCores << " cores)" << endl;
	        }
	        
	        return success;
	    }
	    
	    bool SetProcessPriority(DWORD processId, DWORD priority) {
	        auto it = processes.find(processId);
	        if (it == processes.end()) return false;
	        
	        bool success = SetPriorityClass(it->second->hProcess, priority);
	        if (success) {
	            it->second->priorityClass = priority;
	            cout << "Priority changed for PID " << processId;
	            switch(priority) {
	                case IDLE_PRIORITY_CLASS: cout << " (IDLE)"; break;
	                case BELOW_NORMAL_PRIORITY_CLASS: cout << " (BELOW_NORMAL)"; break;
	                case NORMAL_PRIORITY_CLASS: cout << " (NORMAL)"; break;
	                case ABOVE_NORMAL_PRIORITY_CLASS: cout << " (ABOVE_NORMAL)"; break;
	                case HIGH_PRIORITY_CLASS: cout << " (HIGH)"; break;
	            }
	            cout << endl;
	        }
	        
	        return success;
	    }
	    
	    bool SetLowResourceProfile(DWORD processId) {
	        bool success = true;
	        
	        success &= SetProcessPriority(processId, IDLE_PRIORITY_CLASS);
	        success &= SetProcessCpuAffinity(processId, 1);
	        
	        if (success) {
	            cout << "Low resource profile applied to PID " << processId << endl;
	        }
	        
	        return success;
	    }
	    
	    void SetAllProcessesLowResource() {
	        cout << "Applying low resource profile to all processes..." << endl;
	        for (auto& pair : processes) {
	            if (IsProcessRunning(pair.first)) {
	                SetLowResourceProfile(pair.first);
	            }
	        }
	    }
	    
	    ~ProcessManager() {
	        TerminateAllProcesses(true);
	    }
	// END public
	
	private:
	    bool TerminateProcessInternal(shared_ptr<ProcessInfo> procInfo, bool forceKill) {
	        if (!procInfo || !procInfo->isValid || procInfo->hProcess == NULL) {
	            return false;
	        }
	        
	        DWORD exitCode;
	        if (!GetExitCodeProcess(procInfo->hProcess, &exitCode)) {
	            return false;
	        }
	        
	        if (exitCode != STILL_ACTIVE) {
	            cout << "Process PID " << procInfo->processId << " already terminated." << endl;
	            return true;
	        }
	        
	        bool success = false;
	        
	        if (forceKill) {
	            success = ::TerminateProcess(procInfo->hProcess, 1);
	            if (success) {
	                cout << "Process PID " << procInfo->processId << " force terminated." << endl;
	            }
	        } else {
	            struct EnumData {
	                DWORD processId;
	                bool found;
	            } enumData = { procInfo->processId, false };
	            
	            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
	                EnumData* data = reinterpret_cast<EnumData*>(lParam);
	                DWORD windowProcessId;
	                GetWindowThreadProcessId(hwnd, &windowProcessId);
	                
	                if (windowProcessId == data->processId && IsWindowVisible(hwnd)) {
	                    PostMessage(hwnd, WM_CLOSE, 0, 0);
	                    data->found = true;
	                }
	                return TRUE;
	            }, reinterpret_cast<LPARAM>(&enumData));
	            
	            if (enumData.found) {
	                DWORD waitResult = WaitForSingleObject(procInfo->hProcess, 5000);
	                success = (waitResult == WAIT_OBJECT_0);
	            }
	            
	            if (!success) {
	                cout << "Gentle close failed, using force termination..." << endl;
	                success = ::TerminateProcess(procInfo->hProcess, 1);
	            }
	            
	            if (success) {
	                cout << "Process PID " << procInfo->processId << " terminated." << endl;
	            }
	        }
	        
	        return success;
	    }
	// END private
};

// Funzioni globali per l'utilizzo semplificato
DWORD StartBackgroundProcess(const string& executablePath, const string& arguments = "", 
                            bool lowPriority = false, int limitCpuCores = 0) {
    static ProcessManager manager;
    return manager.StartProcess(executablePath, arguments, lowPriority, limitCpuCores);
}

bool TerminateBackgroundProcess(DWORD processId, bool forceKill = false) {
    static ProcessManager manager;
    return manager.TerminateProcess(processId, forceKill);
}
