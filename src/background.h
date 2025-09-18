#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "utils.h"

inline void rebootPC() {
	system("shutdown /r /t 0");
}
