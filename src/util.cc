
#include "util.h"
#include "Common.h"
#include <iomanip>
#include <fstream>
#include <boost/log/trivial.hpp>
//#include <unordered_set>
#include <vector>


using namespace std;

string recordArr_toStr(const RecordType& rec) {
    stringstream str;

    int fw[] = {10, 13, 16, 18, 6, 2, 1};
    int i = 0;
    for( const auto& field: rec) {
        str << setw(fw[i++]) << left << field;
    }

    return str.str();
}


/// @brief save sorted records into log file for debugging
/// @param strDataArr vector of pair where the first element is the record identifier
///  and the second element is the concatenated record fields.
/// @param recordArr is the record array.
/// @param file_path is the path name for the log file.
void save_sorted_records(vector<pair<int, string> >& strDataArr,
                         vector<vector<string> >& recordArr,
                         const string& file_path) {
    ofstream ofs;
    ofs.open(file_path, ofstream::out);
    for (int i = 0; i < recordArr.size(); ++i) {
        ofs << strDataArr[i].second << "\t";
        ofs << setw(10) << right << strDataArr[i].first << "\t";
        ofs << recordArr_toStr(recordArr[strDataArr[i].first]) << endl;
    }
    ofs.close();
}

void save_blocks(vector<vector<int> >& clusterExactIndArr,
                vector<vector<string> >& recordArr,
                vector<vector<int> >& blockArr,
                const string& file_path) {
    size_t blockTotal = blockArr.size();
    ofstream ofs;
    ofs.open(file_path, ofstream::out);
    BOOST_LOG_TRIVIAL(info) << "Logging blocks to file : "
                            << file_path;
    for(int blockID=0; blockID < blockTotal; ++blockID) {
        if (!blockArr[blockID].empty()) {
            ofs << "Block ID: " << setw(10) << left << blockID << "\t";
            ofs << "has " << setw(4) << left << blockArr[blockID].size() << "records" << endl;
            ofs << endl;
            for(auto& cluster_exact_index: blockArr[blockID]) {
                record_index first_record_index = clusterExactIndArr[cluster_exact_index][0];
                ofs << "\t" << "CI: " << setw(7) << left << cluster_exact_index << "\t";
                ofs << "RI: " << setw(7) << left << first_record_index << "\t";
                for(auto& field: recordArr[first_record_index]) {
                    ofs << setw(15) << left << field << "\t";
                }
                ofs << endl;
            }
            ofs << endl;
        }
    }
    BOOST_LOG_TRIVIAL(info) << "Logging blocks to file done.";
    ofs.close();
}


void save_blocks(vector<vector<int> >& clusterExactIndArr,
                 vector<vector<string> >& recordArr,
                 unordered_map<string, vector<record_index>>& blockArr,
                 const string& file_path) {
    ofstream ofs;
    ofs.open(file_path.c_str(), ofstream::out);
    BOOST_LOG_TRIVIAL(info) << "Logging multi blocks to file : "
                            << file_path.c_str();
    for(const auto& block : blockArr) {
        const auto block_key = block.first;
        const auto block_records = block.second;
        ofs << "Block key: " << setw(10) << left << block_key << "\t";
        ofs << "has " << setw(4) << left << block_records.size() << "records" << endl;
        ofs << endl;
        for(auto& cluster_exact_index: block_records) {
            record_index first_record_index = clusterExactIndArr[cluster_exact_index][0];
            ofs << "\t" << "CI: " << setw(7) << left << cluster_exact_index << "\t";
            ofs << "RI: " << setw(7) << left << first_record_index << "\t";
            for(auto& field: recordArr[first_record_index]) {
                ofs << setw(15) << left << field << "\t";
            }
            ofs << endl;
        }
        ofs << endl;
    }
    BOOST_LOG_TRIVIAL(info) << "Logging blocks to file done.";
    ofs.close();
}


// Function to read CSV file and return dataset as vector of unordered maps
std::vector<std::unordered_map<std::string, std::string>> read_csv(const std::string& filename) {
    std::vector<std::unordered_map<std::string, std::string>> dataset;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        bool header = true;
        std::vector<std::string> headers;
        while (std::getline(file, line)) {
            if (header) {
                headers = split(line, ',');
                header = false;
            } else {
                std::unordered_map<std::string, std::string> row;
                std::vector<std::string> values = split(line, ',');
                for (size_t i = 0; i < headers.size(); ++i) {
                    row[headers[i]] = values[i];
                }
                dataset.push_back(row);
            }
        }
        file.close();
    }
    return dataset;
}

// Function to split a string by delimiter
std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
