

#ifndef RLA_CL_PREFIX_BLOCKING_H
#define RLA_CL_PREFIX_BLOCKING_H

#include "Common.h"
#include <vector>

using namespace std;

static const int ALPHABET_SIZE_LIST[] = {26, 10, 36, 256};

void create_prefix_blocks(int indBlockField, int nChars, int type,
                          vector<vector<int> >& clusterExactIndArr,
                          vector<vector<string> >& recordArr,
                          vector<vector<int> >& indexDatasetArr,
                          vector<vector<int> >& blocks);

#endif //RLA_CL_PREFIX_BLOCKING_H
