

#ifndef RLA_CL_DOUBLEMETAPHONE_BLOCKING_H
#define RLA_CL_DOUBLEMETAPHONE_BLOCKING_H

#include "Common.h"
//#include <unordered_set>
#include <vector>

using namespace std;

void create_doublemetaphone_blocking(multi_blocking_parameters& mbp,
                             vector<vector<int> >& clusterExactIndArr,
                             vector<vector<string> >& recordArr,
                             vector<vector<int> >& indexDatasetArr,
                             unordered_map<string, vector<record_index>>& blockArr,
                             bool log_blocks);

#endif //RLA_CL_DOUBLEMETAPHONE_BLOCKING_H
