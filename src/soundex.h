

#ifndef RLA_CL_SOUNDEX_H
#define RLA_CL_SOUNDEX_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

void soundex(const std::string& name, std::string& soundex_code, int size=4);

#endif //RLA_CL_SOUNDEX_H
