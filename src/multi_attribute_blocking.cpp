//
// Multi Blocking
//

#include "multi_attribute_blocking.h"
#include <vector>
#include <numeric>
#include <boost/log/trivial.hpp>
#include <cmath>

using namespace std;

void create_multi_attribute_blocks(vector<vector<int> >& clusterExactIndArr,
                          vector<vector<string> >& recordArr,
                          vector<vector<int> >& indexDatasetArr,
                          vector<vector<int>>& blockArr)
{
    vector<int> blockingFields{1, 2, 3, 4};  // field indexes for blocking key construction.
    vector<int> blockingCharacters{1, 1, 1, 1};  // number of characters to use for blocking key construction.
    int base = 36;
    assert(blockingFields.size() == blockingCharacters.size());
    int blkKeySize = std::accumulate(blockingCharacters.begin(), blockingCharacters.end(), 0);
    auto blockTotal 	= (unsigned long int) pow(base, blkKeySize);
    blockArr.resize(blockTotal);
    vector<string> record;
    int indFieldDataset, blockID;
    string blockFieldStr;
    string blockKeyStr;
    for (int i = 0; i < clusterExactIndArr.size(); ++i) {
        blockKeyStr.clear();
        record = recordArr[clusterExactIndArr[i][0]];
        auto blkChrsIt = blockingCharacters.begin();
        for (auto blkFldIt = blockingFields.begin(); blkFldIt < blockingFields.end(); ++blkFldIt, ++blkChrsIt) {
            auto index = strtol(record[record.size() - 1].c_str(), nullptr, 10);
            indFieldDataset = indexDatasetArr[index][*blkFldIt];
            if (indFieldDataset < 0)
                continue;
            blockFieldStr = record[indFieldDataset].substr(0, *blkChrsIt);
            blockKeyStr += blockFieldStr;
        }

        vector<int> codeRecordArr;
        auto blkKyL = blockKeyStr.length();  // length of the blocking key

        blockID = 0;
        for (int j = 0; j < blkKyL; ++j) {
            if (blockKeyStr[j] >= 97)
                blockID += (blockKeyStr[j] - 97) * (int) pow(base, j);
            else
                blockID += (blockKeyStr[j] - 48) * (int) pow(base, j);
        }

        if (!(blockID < 0 || blockID >= blockTotal)) {
            blockArr[blockID].push_back(i);
        }
    }
    int blkCount = 0;
    for(auto& b: blockArr){
        if(!b.empty())
            blkCount++;
    }
    BOOST_LOG_TRIVIAL(info) << "Number of non-empty blocks    : " << blkCount;
    BOOST_LOG_TRIVIAL(info) << "Maximum number of blocks      : " << blockTotal;
}
