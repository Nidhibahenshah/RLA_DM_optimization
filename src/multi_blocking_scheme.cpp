//

#include "multi_blocking_scheme.h"
#include "util.h"
#include <boost/log/trivial.hpp>
//#include <unordered_set>
#include <vector>

/// Create multiple blocking scheme based
/// @param mbp vector of multi blocking parameters.
///     each element of @c mbp constitutes an index and a length.
/// @param log_blocks whether to output the blocking information in a log file.
///     This is useful for debugging purposes.
void create_multi_blocking_scheme(multi_blocking_parameters& mbp,
                                  vector<vector<int> >& clusterExactIndArr,
                                  vector<vector<string> >& recordArr,
                                  vector<vector<int> >& indexDatasetArr,
                                  multi_blocking_scheme& blocks,
                                  bool log_blocks)
{
    //BOOST_LOG_TRIVIAL(info) << "Creating Multiple Blocks ........";
    string blkFldIndStr;
    string blkFldLenStr;
    for (auto p : mbp) {
        blkFldIndStr += to_string(p.index) + "  ";
        blkFldLenStr += to_string(p.length) + "  ";
    }
    //BOOST_LOG_TRIVIAL(info) << "\tBlocking Field(s) Index(es)\t:\t" << blkFldIndStr;
    //BOOST_LOG_TRIVIAL(info) << "\tBlocking Field(s) Length(s)\t:\t" << blkFldLenStr;

    unordered_map<string, vector<record_index>> blockArr;
    vector<string> record;
    int indFieldDataset;
    string blockFieldStr;
    string blockKeyStr;
    for (record_index i = 0; i < clusterExactIndArr.size(); ++i) {
        blockKeyStr.clear();
        record = recordArr[clusterExactIndArr[i][0]];
        for (auto p: mbp) {
            indFieldDataset = indexDatasetArr[strtol(record[record.size() - 1].c_str(), nullptr, 10)][p.index];
            if (indFieldDataset < 0)
                continue;
            blockFieldStr = record[indFieldDataset].substr(0, p.length);
            blockKeyStr += blockFieldStr;
        }
        blockArr[blockKeyStr].push_back(i);  // Changed from insert to push_back
    }
    if(log_blocks) {
        save_blocks(clusterExactIndArr, recordArr, blockArr, "MB-blocks");
    }
    int blocking_scheme_index = 0;
    for(const auto& [blk_key, set_records] : blockArr) {
        for(auto it_1=set_records.begin(); it_1!=set_records.end(); ++it_1) {
            for(auto it_2=next(it_1, 1); it_2 != set_records.end(); ++it_2) {
                blocks[{*it_1, *it_2}].insert(blocking_scheme_index);
            }
        }
    }
    //BOOST_LOG_TRIVIAL(info) << " Number of record pairs        : " << blocks.size();
    //BOOST_LOG_TRIVIAL(info) << " Creating Multiple Blocks ........ done";
}

/// Create multiple blocking scheme based
/// @param mbp vector of multi blocking parameters.
///     each element of @c mbp constitutes an index and a length.
/// @param log_blocks whether to output the blocking information in a log file.
///     This is useful for debugging purposes.
void create_multi_blocking_scheme(multi_blocking_parameters& mbp,
                                  vector<vector<int> >& clusterExactIndArr,
                                  vector<vector<string> >& recordArr,
                                  vector<vector<int> >& indexDatasetArr,
                                  unordered_map<string, vector<record_index>>& blockArr,
                                  bool log_blocks)
{
    //BOOST_LOG_TRIVIAL(info) << "Creating Multiple Blocks ........";
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
    for (record_index i = 0; i < clusterExactIndArr.size(); ++i) {
        blockKeyStr.clear();
        record = recordArr[clusterExactIndArr[i][0]];
        for (auto p: mbp) {
            indFieldDataset = indexDatasetArr[strtol(record[record.size() - 1].c_str(), nullptr, 10)][p.index];
            if (indFieldDataset < 0)
                continue;
            blockFieldStr = record[indFieldDataset].substr(0, p.length);
            blockKeyStr += blockFieldStr;
        }
        blockArr[blockKeyStr].push_back(i);  // Changed from insert to push_back
    }

    for (auto& [blk_key, vec_records] : blockArr) {
        sort(vec_records.begin(), vec_records.end());
        vec_records.erase(unique(vec_records.begin(), vec_records.end()), vec_records.end());
    }

    if(log_blocks) {
        save_blocks(clusterExactIndArr, recordArr, blockArr, "MB2-blocks");
    }
    size_t n_rec_pairs = 0;   // Number of record pairs
    for(const auto& [blk_key, vec_records] : blockArr) {  // Changed set_records to vec_records
        auto n_rec = vec_records.size();
        n_rec_pairs += (n_rec * (n_rec - 1)/2);
    }

    //BOOST_LOG_TRIVIAL(info) << " Number of record pairs        : " << n_rec_pairs;
    //BOOST_LOG_TRIVIAL(info) << " Creating Multiple Blocks ........ done";
}


/// Convert multi blocking parameters to string
string to_string(multi_blocking_parameters mbp) {
    stringstream ss;
    for(auto p = mbp.begin(); p!=mbp.end();){
        ss << "I" << p->index << "L" << p->length;
        if(++p!=mbp.end()) ss << " ";
    }
    return ss.str();
}
