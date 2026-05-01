//

#ifndef RLA_CL_MULTI_ATTRIBUTE_BLOCKING_H
#define RLA_CL_MULTI_ATTRIBUTE_BLOCKING_H

#include <vector>
#include <string>

using namespace std;

void create_multi_attribute_blocks(vector<vector<int> >& clusterExactIndArr,
                                   vector<vector<string> >& recordArr,
                                   vector<vector<int> >& indexDatasetArr,
                                   vector<vector<int>>& blockArr);

#endif //RLA_CL_MULTI_ATTRIBUTE_BLOCKING_H
