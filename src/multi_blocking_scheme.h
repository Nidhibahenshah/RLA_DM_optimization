
#ifndef RLA_CL_MULTI_BLOCKING_SCHEME_H
#define RLA_CL_MULTI_BLOCKING_SCHEME_H

#include "Common.h"
//#include <unordered_set>
#include <vector>
using namespace std;

void create_multi_blocking_scheme(multi_blocking_parameters& mbp,
                                  vector<vector<int> >& clusterExactIndArr,
                                  vector<vector<string> >& recordArr,
                                  vector<vector<int> >& indexDatasetArr,
                                  multi_blocking_scheme& blocks,
                                  bool log_blocks=false);

void create_multi_blocking_scheme(multi_blocking_parameters& mbp,
                                  vector<vector<int> >& clusterExactIndArr,
                                  vector<vector<string> >& recordArr,
                                  vector<vector<int> >& indexDatasetArr,
                                  unordered_map<string, vector<record_index>>& blockArr,
                                  bool log_blocks);

string to_string(multi_blocking_parameters mbp);

#endif //RLA_CL_MULTI_BLOCKING_SCHEME_H
