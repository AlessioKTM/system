#pragma once

#include <iostream>
#include <cstdio>
#include <string>
using namespace std;

class Logger {
	public:
		Logger() = default;
		
		void SetFileLog(const string& filepath) {
			string full_filepath = filepath + "\\log.txt";
			where = full_filepath;
			
			FILE* f = fopen(where.c_str(), "a");
			fprintf(f, "# ===== ===== ===== ===== ===== #\n");
			
			fclose(f);
		}
		
		void Log(const string& s) {
			FILE* f = fopen(where.c_str(), "a");
			if (!f) return;
			
			fprintf(f, "%s\n", s.c_str());
			fclose(f);
		}
		
		void ClearLog() {
			FILE* f = fopen(where.c_str(), "w");
			fclose(f);
		}
		
	private:
		string where;
};
