#include "doublemetaphone_blocking.h"
#include "double_metaphone.h"
#include "util.h"
#include <boost/log/trivial.hpp>
#include <array>
//#include <unordered_set>
#include <vector>
#include <set>

/// Create blocking based on doublemetaphone encoding
/// @param mbp vector of multi blocking parameters.
///     each element of @c mbp constitutes an index and a length.
/// @param log_blocks whether to output the blocking information in a log file.
///     This is useful for debugging purposes.
void create_doublemetaphone_blocking(multi_blocking_parameters& mbp,
                                  vector<vector<int> >& clusterExactIndArr,
                                  vector<vector<string> >& recordArr,
                                  vector<vector<int> >& indexDatasetArr,
                                  unordered_map<string, vector<record_index>>& blockArr,
                                  bool log_blocks)
{
    //BOOST_LOG_TRIVIAL(info) << "Creating Double Metaphone Blocks ........";
    string blkFldIndStr;
    string blkFldLenStr;
    for (auto p : mbp) {
        blkFldIndStr += to_string(p.index) + "  ";
        blkFldLenStr += to_string(p.length) + "  ";
    }
    //BOOST_LOG_TRIVIAL(info) << "\tBlocking Field(s) Index(es)\t:\t" << blkFldIndStr;
    //BOOST_LOG_TRIVIAL(info) << "\tBlocking Field(s) Length(s)\t:\t" << blkFldLenStr;

    vector<string> record;
    int indFieldDataset;
    string blockFieldStr;
    string blockKeyStr;
    //vector<string> doublemetaphone_code;
    //std::array<std::string, 2> doublemetaphone_code;
    std::string doublemetaphone_code; // Changed to single string
    set<string> uniqueNames;  // To store unique blocking field values
    
    for (record_index i = 0; i < clusterExactIndArr.size(); ++i) {
        blockKeyStr.clear();
        record = recordArr[clusterExactIndArr[i][0]];
        for (auto p: mbp) {
            indFieldDataset = indexDatasetArr[strtol(record[record.size() - 1].c_str(), nullptr, 10)][p.index];
            if (indFieldDataset < 0)
                continue;
            blockFieldStr = record[indFieldDataset];        // .substr(0, p.length);
            //doublemetaphone_code.clear();
            uniqueNames.insert(blockFieldStr);  // Collect unique names
            
            DoubleMetaphone(blockFieldStr, &doublemetaphone_code);

            // Concatenate all codes into the block key
            for (const auto& code : doublemetaphone_code) {
                //blockKeyStr += doublemetaphone_code[0];
                blockKeyStr += doublemetaphone_code; // Use single code directly
            }

//            blockKeyStr += doublemetaphone_code;
        }
        blockArr[blockKeyStr].push_back(i);  // Changed from insert to push_back
    }

    // Deduplicate records in each block to mimic unordered_set behavior
    for (auto& [blk_key, vec_records] : blockArr) {
        sort(vec_records.begin(), vec_records.end());  // Sort for uniqueness
        vec_records.erase(unique(vec_records.begin(), vec_records.end()), vec_records.end());  // Remove duplicates
    }

    if(log_blocks) {
        save_blocks(clusterExactIndArr, recordArr, blockArr, "Double Metaphone-blocks");
    }
    size_t n_rec_pairs = 0;   // Number of record pairs
    for(const auto& [blk_key, vec_records] : blockArr) {  // Changed set_records to vec_records
        auto n_rec = vec_records.size();  // Vector uses size() just like unordered_set
        n_rec_pairs += (n_rec * (n_rec - 1)/2);
    }

    BOOST_LOG_TRIVIAL(info) << " Number of record pairs        : " << n_rec_pairs;
    BOOST_LOG_TRIVIAL(info) << " Creating Double Metaphone Blocks ........ done";
    BOOST_LOG_TRIVIAL(info) << " Number of unique names in blocking fields: " << uniqueNames.size();
}
