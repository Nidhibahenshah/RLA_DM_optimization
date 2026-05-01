/*
 * Cluster.h
 */

#ifndef CLUSTER_H_
#define CLUSTER_H_

#include "Common.h"
#include <vector>
#include <string>

using namespace std;

class Cluster
{
	public:
		float height;
		vector<vector<string> > itemArr;

		void initCluster(float height, vector<string> item);
		void initCluster(float height, Cluster c1, Cluster c2);
		vector<string> getRepresentative();
		void printCluster();
};

class cluster{

public:
    cluster_id id; //cluster identifier
    records_t records; //records in this cluster
    cluster();
    cluster(cluster_id Id, records_t Records);
    count_t get_number_of_records();
    void clear();
};

typedef std::vector<cluster> clusters;

#endif



