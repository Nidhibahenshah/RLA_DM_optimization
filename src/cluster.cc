/*
 * Cluster.cpp
 */


// library
#include <iostream>
#include <vector>

#include "cluster.h"


// namespace
using namespace std;

void Cluster::initCluster(float height, vector<string> item)
{
	this->height = height;
	itemArr.push_back(item);
}

void Cluster::initCluster(float height, Cluster c1, Cluster c2)
{
	this->height = height;
	itemArr = c1.itemArr;
	itemArr.insert(itemArr.end(), c2.itemArr.begin(), c2.itemArr.end());
}

vector<string> Cluster::getRepresentative()
{
	return itemArr.at(0);
}

void Cluster::printCluster()
{

}

cluster::cluster(){
  this->id=0;
  this->records=records_t();
}

cluster::cluster(cluster_id Id, records_t Records){
  this->id=Id;
  this->records=records_t(Records);
}

count_t cluster::get_number_of_records(){
  return(this->records.size());
}

void cluster::clear(){
  this->id=0;
  this->records.clear();
}
