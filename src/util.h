#ifndef RLA_CL_UTIL_H
#define RLA_CL_UTIL_H

#include "Common.h"
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
//#include <unordered_set>
#include <vector>

using namespace std;

typedef vector<string> RecordType;

std::string recordArr_toStr(const RecordType& rec);
void save_sorted_records(vector<pair<int, string> >& strDataArr,
                         vector<vector<string> >& recordArr,
                         const string& file_path);

void save_blocks(std::vector<std::vector<int> >& clusterExactIndArr,
                std::vector<std::vector<std::string> >& recordArr,
                std::vector<std::vector<int> >& blockArr,
                const std::string& file_path);

void save_blocks(vector<vector<int> >& clusterExactIndArr,
                 vector<vector<string> >& recordArr,
                 unordered_map<string, vector<record_index>>& blockArr,
                 const string& file_path);

std::vector<std::unordered_map<std::string, std::string>> read_csv(const std::string& filename);
std::vector<std::string> split(const std::string &s, char delimiter);

#endif //RLA_CL_UTIL_H
