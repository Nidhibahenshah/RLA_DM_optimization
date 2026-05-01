
#include "prefix_blocking.h"
#include <boost/log/trivial.hpp>
#include <cmath>

using namespace std;

/// @brief Create blocks based on first n characters
/// @param indBlockField index of the blocking field
/// @param nChars number of characters to be used for blocking
/// @param type blocking field type 0: alphabet, 1: numeric, or 2: alphanumeric
/// @param blocks blocks created
void create_prefix_blocks(int indBlockField, int nChars, int type,
                          vector<vector<int> >& clusterExactIndArr,
                          vector<vector<string> >& recordArr,
                          vector<vector<int> >& indexDatasetArr,
                          vector<vector<int> >& blocks)
{
    int base = ALPHABET_SIZE_LIST[type];
    auto n_blocks_max 	= pow(base, nChars);
    blocks.resize(n_blocks_max);
    BOOST_LOG_TRIVIAL(info) << "Blocking based on first " << nChars << " characters";
    BOOST_LOG_TRIVIAL(info) << "  Blocking field index : " << indBlockField;
    BOOST_LOG_TRIVIAL(info) << "  Base  : " << base;
    BOOST_LOG_TRIVIAL(info) << "  Maximum number of blocks : " << n_blocks_max;
    vector<string> record;
    int indFieldDataset, blockID;
    string blockFieldStr;
    int blkCount = 0;
    string strSample;
    if (type == 0) // alphabet is english alphabet
        strSample	= "aaaaaaaaaa"; // enough amount of characters for empty string (here 10)
    else
        strSample	= "0000000000";
    for (int i = 0; i < clusterExactIndArr.size(); ++i) {
        record	= recordArr[clusterExactIndArr[i][0]];
        indFieldDataset	= indexDatasetArr[atoi(record[record.size() - 1].c_str())][indBlockField];
        if (indFieldDataset < 0)
            continue;
        blockFieldStr	= record[indFieldDataset];
        size_t strLen	= blockFieldStr.length();
        if (strLen < nChars) {
            blockFieldStr	= strSample.substr(0, nChars - strLen) + blockFieldStr;
        }
        vector<int> codeRecordArr;
        codeRecordArr.clear();
        for (int j = 0; j < nChars; ++j)
            codeRecordArr.push_back((int) blockFieldStr[j]);
        blockID=0;
        for (int k = 0; k < nChars; ++k) {
            int w = (int) pow(ALPHABET_SIZE_LIST[type], 2 - k);
            if (type == 0)
                blockID += (codeRecordArr[k] - 97) * w;
            else if (type == 1)
                blockID += (codeRecordArr[k] - 48) * w;
            else if (type == 2) {
                if (codeRecordArr[k] >= 97)
                    blockID += (codeRecordArr[k] - 97) * w;
                else
                    blockID += (codeRecordArr[k] - 22) * w; // 48 - 26
            }
        }
        assert(!(blockID < 0 || blockID >= n_blocks_max));
        blocks[blockID].push_back(i);
    }
    for(auto& b: blocks)
        if(!b.empty())
            blkCount++;
    BOOST_LOG_TRIVIAL(info) << "  Number of non-empty blocks : " << blkCount;
}
