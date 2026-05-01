#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <vector>

typedef unsigned long long int record_index;
typedef unsigned int cluster_id;
typedef unsigned long long int record_index;

typedef std::vector<std::string> record_t;
typedef std::vector<record_t> records_t;
typedef unsigned long long int count_t;

typedef struct
{
	std::string str;
	int ind;
}StrPacket;

typedef struct LinkParamT
{
    int n_datasets = 0;
    std::string datasets_file_names = std::string(256, '\0');
    long n_records_total_read = 0;
    int threshold = 0;
    double linkage_time_in_seconds = -1;
    double precision = -0.1;
    double recall = -0.1;
}LinkParam;

// Blocking related definitions
typedef std::pair<record_index, record_index> record_pair;
typedef std::unordered_map<record_pair, std::set<int>, boost::hash<record_pair>> multi_blocking_scheme;
struct multi_blocking_paramT {
    int index;
    int length;
};
typedef std::vector<multi_blocking_paramT> multi_blocking_parameters;

#endif /* COMMON_H_ */
