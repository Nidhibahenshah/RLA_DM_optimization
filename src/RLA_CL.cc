///
/// @file: RLA_CL.cpp


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <set>
//#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <cmath>
#include <utility>
#include <locale>
#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iomanip>

#include "cluster.h"
#include "Common.h"
#include "SortMSD.h"
#include "RLA_CL.h"
#include "performance.h"
#include "util.h"
#include "multi_attribute_blocking.h"
#include "edit_distance.h"
#include "multi_blocking_scheme.h"
#include "soundex_blocking.h"
#include <boost/functional/hash.hpp>
#include "union_find.h"
#include "doublemetaphone_blocking.h"

// namespaces
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

// MACROS
#define LINE_LENGTH_MIN		10
#define TYPE_EDIT			0
#define TYPE_REVERSAL		1
#define TYPE_TRUNC			2
#define TYPE_TOTAL			7
#define TYPE_QGRAM			5
#define TYPE_HAUSDORFF		6
#define OUTPUT_FINAL		0
#define OUTPUT_SINGLE		1

typedef enum {
    BLOCKING_TYPE_UNKNOWN   =   0,
    BLOCKING_TYPE_KMER      =   1,
    BLOCKING_TYPE_MULTI     =   2,
    BLOCKING_TYPE_MULTI_2   =   3,
    BLOCKING_TYPE_SOUNDEX   =   4,
    BLOCKING_TYPE_DOUBLEMETAPHONE = 5
} BLOCKING_TYPE;

typedef struct BLOCKING_STAGE_TAG{
    BLOCKING_TYPE blockingType=BLOCKING_TYPE_UNKNOWN;
    vector<int> index;
    vector<int> kmer;
    vector<int> type;
    vector<int> length;
//    vector<int> length;
    BLOCKING_STAGE_TAG() {
        blockingType = BLOCKING_TYPE_UNKNOWN;
    }
} BLOCKING_STAGE;

vector<BLOCKING_STAGE> blockingStages;

void getKmerBlockingParameters(xmlXPathObject *pathObj, BLOCKING_STAGE& bs);
void getMultiBlockingParameters(xmlXPathObject *pathObj, BLOCKING_STAGE& bs);
void getBlockingStages(vector<BLOCKING_STAGE>& blockingStages);

vector<int> getWeight();
vector<vector<int> > getIndexDataSet();
vector<string> getInputFileNameList();
float getInputThreshold();
vector<int> getPriorityField();
vector<vector<vector<int> > > getInputComparisonPara();
string getOutputDir();
string getResultsLogFile();

void readDataFromFile(vector<string>& fileNameArr);

void clusterData(LinkParam& lp);
void sortData();
void findExactClusterSL();
void findExactClusterEach();
void findExactCluster();
void appendExactInd(int pos, vector<int>& tempArr);
void findExactClusterPart(int i, int j);
void findApproxCluster(vector<int>& rootArr);
void findFinalCluster(vector<int>& rootArr);

void createBlock(int indBlockField, int kmer, int type, vector<vector<int> >& blockArr);
void createClusterEdgeList(vector<vector<int> >& blockArr);
void generateEdgeList(vector<int>& blockRowArr);

bool isLinkageOk(vector<string>& a, vector<string>& b);
double linkage(vector<string>& a, vector<string>& b);
double calculateDistAll(vector<string>& a, vector<string>& b);
int calculateEditDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt, int threshRem);
int calculateRevDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt, int threshRem);
int calculateTruncDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt, int threshRem);

double calculateQGramDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt, int k);
double calculateBasicQgram(string& str1, string& str2,int threshRem);
set<string> generateKmers(string& str,int k);

double calculateHausdorffDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt,int k);
double calculateBasicHausdorffDistance(string& str1,string& str2,int threshRem);
int calculateHammingDistance(string& str1,string& str2);

void clusterGrp(vector<int>& recordIndSingleGrpArr, vector<Cluster>& clusterSingleArr);
void generateMatrix(vector<vector<string> >& recordGrpArr, vector<vector<int> >& matArr);
void generateVector(vector<vector<int> >& matArr, vector<vector<int> >& vecArr);
void updateMatVec(vector<vector<int> > &matArr, vector<vector<int> > &vecArr, int indCluster1, int indCluster2);
void postprocessCluster(vector<Cluster>& clusterSingleArr);
bool isInCluster(Cluster& cluster, vector<string>& record);
void processUsedAttr(vector<vector<int> >& usedAttrArr, vector<int>& isInClusterArr, int indDataset, vector<vector<int> >& usedThisAttrArr);
void output(int outputType, LinkParam& lp);

static const int ALPHABET_SIZE_LIST[] = {26, 10, 36, 256};

clock_t startT;
double readT, clusterExactT, clusterApproxT, clusterCombineT, clusterCLT, totalT, outSLT, outCLT;
int recordTotal, recordTotalCMD;
string configFileStr, outDir;
float threshold;

vector<Cluster> clusterArr;
vector<string> fileNameArr;
vector<vector<string> > recordArr;
vector<int> weightArr, priorityFieldArr, recordStartIndArr;
vector<vector<int> > edgeArr, clusterExactIndArr, indexDatasetArr, clusterIndArr, clusterExactPairArr;
vector<vector<vector<int> > > attrArr;

int lenMax;
int distance_function = 0;
edit_distance ed = edit_distance();

vector<pair<int, string>> strDataArr;
bool isCompleteLinkageEnabled = false;
bool isBlocksLoggingEnabled = false;
bool isLimitRecordsEnabled = false;

bool filesExists (const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

string algorithm_version_signature;

void doRLA(int ac, char** av)
{
  startT		= clock();
  algorithm_version_signature = "RLA2";
    BOOST_LOG_TRIVIAL(info) << algorithm_version_signature;

    // Declare the supported options.
    po::options_description desc("Linking Configuration Settings");
    desc.add_options()
            ("help", "displays help message")
            ("log,L", po::value<string>(),
                    "set file name for logging results")
            ("complete-linkage,C", po::value<bool>(),
                    "Whether to apply complete linkage")
            ("output-blocks", po::value<bool>(),
                    "Whether to output the blocking information to a log file.")
            ("limit-records", po::value<int>(),
                    "Limit the total number of records to be read from input dataset files."
                    "RLA2 parses same number of records from all files.")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        exit(EXIT_SUCCESS);
    }
    if (vm.count("complete-linkage")) {
        isCompleteLinkageEnabled = vm["complete-linkage"].as<bool>();
    }
    if (isCompleteLinkageEnabled) {
        BOOST_LOG_TRIVIAL(info) << "Complete Linkage has been enabled";
    } else {
        BOOST_LOG_TRIVIAL(warning) << "Complete Linkage has been disabled";
    }
    if (vm.count("output-blocks")) {
        isBlocksLoggingEnabled = vm["output-blocks"].as<bool>();
    }
    if (vm.count("limit-records")) {
        isLimitRecordsEnabled = true;
        recordTotalCMD = vm["limit-records"].as<int>();
        BOOST_LOG_TRIVIAL(warning) << "Reading only " << recordTotalCMD << " records in total";
    }

    std::stringstream commandLineInvoked;
    for (int ii=0; ii < ac; ++ii) {
        if (ii > 0) {
            commandLineInvoked << " ";
        }
        commandLineInvoked << av[ii];
    }
    BOOST_LOG_TRIVIAL(info) << "Command line invoked : "
        << commandLineInvoked.str();
    if (ac < 2) {
        BOOST_LOG_TRIVIAL(fatal) << "rla_cl config_file_name.xml";
        exit(EXIT_FAILURE);
    }
    string fileAddr = av[1];

    // read data from configuration file
	configFileStr	= fileAddr;

	weightArr		= getWeight();
	indexDatasetArr	= getIndexDataSet();
	fileNameArr		= getInputFileNameList();
    getBlockingStages(blockingStages);
	priorityFieldArr= getPriorityField();
	threshold		= getInputThreshold();
	attrArr			= getInputComparisonPara();
    outDir			= getOutputDir();
	// read data from input file
    clock_t readT_start = clock();
	readDataFromFile(fileNameArr);
	readT		= (double)(clock() - readT_start) / CLOCKS_PER_SEC;
    BOOST_LOG_TRIVIAL(info) << "Number of records read        : " << recordArr.size();
    BOOST_LOG_TRIVIAL(info) << "Threshold value               : " << threshold;
    std::stringstream datasets_file_names;
    for (size_t k=0; k<fileNameArr.size(); ++k) {
        if (k>0) {
            datasets_file_names << ";";
        }
        datasets_file_names << fileNameArr[k];
    }
    LinkParam lp;
    lp.n_records_total_read = recordArr.size();
    lp.datasets_file_names = datasets_file_names.str();
    lp.n_datasets = fileNameArr.size();
    lp.threshold = threshold;
    // get local thresholds if any
    vector<pair<int, double> > local_thresholds;
    vector<vector<int> > edit_attrArr = attrArr[0];
    stringstream thresholds;
    for(int ii = 0; ii<edit_attrArr.size(); ++ii) {
        int index = edit_attrArr[ii][0];
        int local_threshold = edit_attrArr[ii][1];
        if (local_threshold>0) {
            local_thresholds.push_back({index, local_threshold});
            thresholds << "I" << index << "T" << local_threshold << " ";
        }
    }
    if(!local_thresholds.empty()) {
        stringstream lt_idx;
        stringstream lt_tld;
        for(auto lt : local_thresholds) {
            lt_idx << lt.first << "\t";
            lt_tld << lt.second << "\t";
        }
        BOOST_LOG_TRIVIAL(info) <<  " Local Threshold Index(es)    : " << lt_idx.str();
        BOOST_LOG_TRIVIAL(info) <<  " Local Threshold Values       : " << lt_tld.str();
    }


    // cluster data
    clusterData(lp);

  double linkage_time_sl = clusterExactT + clusterApproxT + clusterCombineT;
  double linkage_time_cl;
  if(isCompleteLinkageEnabled) {
      linkage_time_cl = linkage_time_sl + clusterCLT;
  }

  // Load single linkage output file and evaluate linkage performance
  string outFileName = getOutputDir();
  outFileName.append("OutSingle");
  BOOST_LOG_TRIVIAL(info) << "Loading single linkage output ...";
  clock_t loadLinkageOutputStartT		= clock();
  clusters cs = loadLinkageOutput(outFileName);
  clock_t loadLinkageOutputT	= (clock() - loadLinkageOutputStartT) / CLOCKS_PER_SEC;
  BOOST_LOG_TRIVIAL(info) << "\tdone in " << loadLinkageOutputT << " seconds";
  clock_t perfEvalStartT = clock();
  BOOST_LOG_TRIVIAL(info) << "Evaluating single linkage performance ...";
  performance perfSL = evaluateLinkagePerformance(cs, 0, outFileName.append("_detailed_performance"));
  clock_t perfEvalT = (double)(clock() - perfEvalStartT) / CLOCKS_PER_SEC;
  BOOST_LOG_TRIVIAL(info) << "\tdone in " << perfEvalT << " seconds";

  performance perfCL;

  if(isCompleteLinkageEnabled) {
      // Load complete linkage output file and evaluate linkage performance
      outFileName = getOutputDir();
      outFileName.append("OutFinal");
      BOOST_LOG_TRIVIAL(info) << "Loading complete linkage output ...";
      loadLinkageOutputStartT = clock();
      cs = loadLinkageOutput(outFileName);
      loadLinkageOutputT = (clock() - loadLinkageOutputStartT) / CLOCKS_PER_SEC;
      BOOST_LOG_TRIVIAL(info) << "\tdone in " << loadLinkageOutputT << " seconds";
      perfEvalStartT = clock();
      BOOST_LOG_TRIVIAL(info) << "Evaluating complete linkage performance ...";
      perfCL = evaluateLinkagePerformance(cs, 0, outFileName.append("_detailed_performance"));
      perfEvalT = (clock() - perfEvalStartT) / CLOCKS_PER_SEC;
      BOOST_LOG_TRIVIAL(info) << "\tdone in " << perfEvalT << " seconds";
  }

  cout << "Single linkage performance: " << endl;
  printPerformance(perfSL, cout);

  if(isCompleteLinkageEnabled) {
      cout << "Complete Linkage Performance: " << endl;
      printPerformance(perfCL, cout);
  }

  // Compile single linkage results
  std::stringstream resultsSL;
  resultsSL << algorithm_version_signature + " SL"
          << "," << lp.threshold << " " << thresholds.str()
          << "," << lp.n_records_total_read
          << "," << linkage_time_sl
          << "," << perfSL.f1_score * 100.0
          << "," << perfSL.precision * 100.0
          << "," << perfSL.recall * 100.0
          << "," << perfSL.b
          << "," << perfSL.c
          << "," << perfSL.d
          << "," << perfSL.a
          << "," << perfSL.nc
          << "," << lp.n_datasets
          << "," << lp.datasets_file_names
          << "," << commandLineInvoked.str()
          << std::endl;

  std::stringstream resultsCL;
  if(isCompleteLinkageEnabled) {
// Compile complete linkage results
      resultsCL << algorithm_version_signature + " CL"
                << "," << lp.threshold << " " << thresholds.str()
                << "," << lp.n_records_total_read
                << "," << linkage_time_cl
                << "," << perfCL.f1_score * 100.0
                << "," << perfCL.precision * 100.0
                << "," << perfCL.recall * 100.0
                << "," << perfCL.b
                << "," << perfCL.c
                << "," << perfCL.d
                << "," << perfCL.a
                << "," << perfCL.nc
                << "," << lp.n_datasets
                << "," << lp.datasets_file_names
                << "," << commandLineInvoked.str()
                << std::endl;
  }
  std::string results_logging_file_name = (vm.count("log")) ? vm["log"].as<string>() : getResultsLogFile();
  if(results_logging_file_name.empty())
    results_logging_file_name = "results.csv";
  ofstream log;
  if(!fs::exists(results_logging_file_name))
  {
    ofstream log(results_logging_file_name, std::ios_base::app | std::ios_base::out);
    log << "Algorithm"
        << ",Threshold(s)"
        << ",Records"
        << ",LinkageTime(s)"
        << ",F1Score(%)"
        << ",Precision(%)"
        << ",Recall(%)"
        << ",FalseMatches"
        << ",FalseNonMatches"
        << ",TrueMatches"
        << ",TrueNonMatches"
        << ",NumberOfClusters"
        << ",NumberOfDatasets"
        << ",Datasets"
        << ",CommandLineInvoked"
        << std::endl;
    log << resultsSL.str();
    if(isCompleteLinkageEnabled) {
        log << resultsCL.str();
    }
  }
  else
  {
    ofstream log(results_logging_file_name, std::ios_base::app | std::ios_base::out);
    log << resultsSL.str();
    if(isCompleteLinkageEnabled) {
      log << resultsCL.str();
    }
  }

  BOOST_LOG_TRIVIAL(info) << "Total Number of records                : " << recordTotal;
  totalT			= (clock() - startT) / CLOCKS_PER_SEC;
  BOOST_LOG_TRIVIAL(info) << "Total time (s)                         : " << setprecision(1) << fixed << totalT;
  BOOST_LOG_TRIVIAL(info) << "Reading time (s)                       : " << setprecision(1) << fixed << readT;
  BOOST_LOG_TRIVIAL(info) << "Writing SL output time (s)             : " << setprecision(1) << fixed << outSLT;
  if(isCompleteLinkageEnabled) {
      BOOST_LOG_TRIVIAL(info) << "Writing CL output time (s)             : " << setprecision(1) << fixed << outCLT;
  }
  BOOST_LOG_TRIVIAL(info) << "Exact clustering time (s)              : " << setprecision(1) << fixed << clusterExactT;
  BOOST_LOG_TRIVIAL(info) << "Approximate clustering time (s)        : " << setprecision(1) << fixed << clusterApproxT;
  BOOST_LOG_TRIVIAL(info) << "Combining Exact & Approx. time (s)     : " << setprecision(1) << fixed << clusterCombineT;
  if(isCompleteLinkageEnabled) {
      BOOST_LOG_TRIVIAL(info) << "Complete linkage clustering time (s)   : " << setprecision(1) << fixed << clusterCLT;
      BOOST_LOG_TRIVIAL(info) << "Precision                              : " << setprecision(2) << fixed
                              << "SL(" << perfSL.precision * 100.0 << " %) " << setprecision(2) << fixed
                              << "CL(" << perfCL.precision * 100.0 << " %)";
      BOOST_LOG_TRIVIAL(info) << "Recall                                 : " << setprecision(2) << fixed
                              << "SL(" << perfSL.recall * 100.0 << " %) " << setprecision(2) << fixed
                              << "CL(" << perfCL.recall * 100.0 << " %)";
      BOOST_LOG_TRIVIAL(info) << "F1 Score                               : " << setprecision(2) << fixed
                              << "SL(" << perfSL.f1_score * 100.0 << " %) " << setprecision(2) << fixed
                              << "CL(" << perfCL.f1_score * 100.0 << " %)";
      BOOST_LOG_TRIVIAL(info) << "Linkage time (s)                       : " << setprecision(1) << fixed
                              << "SL(" << linkage_time_sl << " ) " << setprecision(1) << fixed
                              << "CL(" << linkage_time_cl << " )";
  } else {
      BOOST_LOG_TRIVIAL(info) << "Precision                              : " << setprecision(2) << fixed
                              << "SL(" << perfSL.precision * 100.0 << " %) " << setprecision(2) << fixed;
      BOOST_LOG_TRIVIAL(info) << "Recall                                 : " << setprecision(2) << fixed
                              << "SL(" << perfSL.recall * 100.0 << " %) " << setprecision(2) << fixed;
      BOOST_LOG_TRIVIAL(info) << "F1 Score                               : " << setprecision(2) << fixed
                              << "SL(" << perfSL.f1_score * 100.0 << " %) " << setprecision(2) << fixed;
      BOOST_LOG_TRIVIAL(info) << "Linkage time (s)                       : " << setprecision(1) << fixed
                              << "SL(" << linkage_time_sl << " ) " << setprecision(1) << fixed;
  }
}

// read weight parameters from config xml
vector<int> getWeight()
{
	xmlDoc *doc 				= xmlReadFile(configFileStr.c_str(), nullptr, 0);
	xmlXPathContext *pathCtx 	= xmlXPathNewContext(doc);
	xmlXPathObject *pathObj 	= xmlXPathEvalExpression((xmlChar *)"/rla-config/weights", pathCtx);
	xmlXPathFreeContext(pathCtx);

	vector<int> weights;
	for (int i = 0; i < pathObj->nodesetval->nodeNr; ++i)
	{
		xmlNode *nodeCurr 	= pathObj->nodesetval->nodeTab[i];
		xmlNode *nodeChild 	= nodeCurr->children;

		while(nodeChild)
		{
			if(!xmlStrcmp(nodeChild->name, (xmlChar *)"value"))
			{
				string weight = (char *)xmlNodeListGetString(nodeCurr->doc, nodeChild->children, 1);
				vector<string> valStrArr;
				split(valStrArr, weight, is_any_of(" "));
				for(vector<string>::iterator it = valStrArr.begin(); it != valStrArr.end(); ++it)
					weights.push_back(atoi((*it).c_str()));
				break;
			}
			nodeChild = nodeChild->next;
		}
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return weights;
}


/// Read index of attributes of data sets from config xml
vector<vector<int> > getIndexDataSet()
{
	xmlDoc *doc 				= xmlReadFile(configFileStr.c_str(), nullptr, 0);
	xmlXPathContext *pathCtx 	= xmlXPathNewContext(doc);
	xmlXPathObject *pathObj 	= xmlXPathEvalExpression((xmlChar *)"/rla-config/dataset/dataset_index", pathCtx);
	xmlXPathFreeContext(pathCtx);
	vector<vector<int> > indexArr;
	for (int i = 0; i < pathObj->nodesetval->nodeNr; ++i)
	{
		xmlNode *nodeCurr = pathObj->nodesetval->nodeTab[i];
		xmlNode *nodeChild = nodeCurr->children;
		while(nodeChild)
		{
			if(!xmlStrcmp(nodeChild->name, (xmlChar *)"value"))
			{
				string weight = (char *)xmlNodeListGetString(nodeCurr->doc, nodeChild->children, 1);
				vector<string> valStrArr;
				split(valStrArr, weight, is_any_of(" "));
				vector<int> indexRowArr;
				for(auto it = valStrArr.begin(); it != valStrArr.end(); ++it)
					indexRowArr.push_back(strtol((*it).c_str(), nullptr, 10));
				indexArr.push_back(indexRowArr);
				break;
			}
			nodeChild = nodeChild->next;
		}
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return indexArr;
}

/// Read input file names from config xml
vector<string> getInputFileNameList()
{
	xmlDoc *doc					= xmlReadFile(configFileStr.c_str(), nullptr, 0);
	xmlXPathContext *xpathCtx	= xmlXPathNewContext(doc);
	xmlXPathObject *xpathObj	= xmlXPathEvalExpression((xmlChar *)"/rla-config/dataset", xpathCtx);
	xmlXPathFreeContext(xpathCtx);

	vector<string> fileListArr;
	for (int i = 0; i < xpathObj->nodesetval->nodeNr; ++i)
	{
		xmlNode *nodeCurr	= xpathObj->nodesetval->nodeTab[i];
		xmlNode *nodeChild	= nodeCurr->children;
		while (nodeChild)
		{
			if(!xmlStrcmp(nodeChild->name, (xmlChar *)"value"))
			{
				fileListArr.push_back((char *)xmlNodeListGetString(nodeCurr->doc, nodeChild->children, 1));
				break;
			}
			nodeChild	= nodeChild->next;
		}
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return fileListArr;
}


/// Read threshold value from config xml
float getInputThreshold()
{
	xmlDoc *doc					= xmlReadFile(configFileStr.c_str(), nullptr, 0);
	xmlXPathContext *xpathCtx	= xmlXPathNewContext(doc);
	xmlXPathObject *xpathObj	= xmlXPathEvalExpression((xmlChar *)"/rla-config/version-config-param/threshold", xpathCtx);
	xmlXPathFreeContext(xpathCtx);
	auto *attrVal	= (xmlChar *)"";
	xmlNode *nodeCurr	= xpathObj->nodesetval->nodeTab[0];
	xmlNode *nodeChild	= nodeCurr->children;

	while (nodeChild)
	{
		if(!xmlStrcmp(nodeChild->name, (xmlChar *)"value"))
		{
			attrVal	= xmlNodeListGetString(nodeCurr->doc, nodeChild->children, 1);
			break;
		}
		nodeChild	= nodeChild->next;
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return atof((char *)attrVal);
}


void getKmerBlockingParameters(xmlXPathObject *pathObj, BLOCKING_STAGE& bs) {
    bs.blockingType = BLOCKING_TYPE_KMER;
    vector<vector<int> > kmerBlockingParameters(3);
    for (int i = 0; i < pathObj->nodesetval->nodeNr; ++i)
    {
        xmlNode *nodeCurr = pathObj->nodesetval->nodeTab[i];
        xmlNode *nodeChild = nodeCurr->children;

        while(nodeChild)
        {
            if(!xmlStrcmp(nodeChild->name, (xmlChar *)"index"))
            {
                xmlNode *valIndex	= nodeChild->children;
                while(valIndex)
                {
                    if(!xmlStrcmp(valIndex->name, (xmlChar *)"value"))
                    {
                        string index = (char *)xmlNodeListGetString(nodeCurr->doc, valIndex->children, 0);
                        vector<string> valStrArr;
                        split(valStrArr, index, is_any_of(","));
                        vector<int> indexRowArr;
                        for(auto & it : valStrArr)
                            bs.index.push_back(atoi(it.c_str()));
                        break;
                    }
                    valIndex = valIndex->next;
                }
            }
            else if(!xmlStrcmp(nodeChild->name, (xmlChar *)"kmer"))
            {
                xmlNode *valLength	= nodeChild->children;
                while(valLength)
                {
                    if(!xmlStrcmp(valLength->name, (xmlChar *)"value"))
                    {
                        string length = (char *)xmlNodeListGetString(nodeCurr->doc, valLength->children, 0);
                        vector<string> valStrArr;
                        split(valStrArr, length, is_any_of(","));
                        vector<int> lengthArr;
                        for(auto & it : valStrArr)
                            bs.kmer.push_back(atoi(it.c_str()));
                        break;
                    }
                    valLength = valLength->next;
                }
            }
            else if(!xmlStrcmp(nodeChild->name, (xmlChar *)"type"))
            {
                xmlNode *valType	= nodeChild->children;
                while (valType)
                {
                    if(!xmlStrcmp(valType->name, (xmlChar *)"value"))
                    {
                        string type = (char *)xmlNodeListGetString(nodeCurr->doc, valType->children, 0);
                        vector<string> valStrArr;
                        split(valStrArr, type, is_any_of(","));
                        vector<int> typeArr;
                        for(auto & it : valStrArr)
                            bs.type.push_back(atoi(it.c_str()));
                        break;
                    }
                    valType = valType->next;
                }
            }
            nodeChild = nodeChild->next;
        }
    }
}

void getMultiBlockingParameters(xmlXPathObject *pathObj, BLOCKING_STAGE& bs) {
    bs.blockingType = BLOCKING_TYPE_MULTI;
    for (int i = 0; i < pathObj->nodesetval->nodeNr; ++i)
    {
        xmlNode *nodeCurr = pathObj->nodesetval->nodeTab[i];
        xmlNode *nodeChild = nodeCurr->children;

        while(nodeChild)
        {
            if(!xmlStrcmp(nodeChild->name, (xmlChar *)"field"))
            {
                xmlNode *valIndex	= nodeChild->children;
                while(valIndex)
                {
                    if(!xmlStrcmp(valIndex->name, (xmlChar *)"index"))
                    {
                        string index = (char *)xmlNodeListGetString(nodeCurr->doc, valIndex->children, 0);
                        vector<string> valStrArr;
                        split(valStrArr, index, is_any_of(","));
                        vector<int> indexRowArr;
                        for(auto & it : valStrArr)
                            bs.index.push_back(atoi(it.c_str()));
                    }
                    else if(!xmlStrcmp(valIndex->name, (xmlChar *)"length"))
                    {
                        string index = (char *)xmlNodeListGetString(nodeCurr->doc, valIndex->children, 0);
                        vector<string> valStrArr;
                        split(valStrArr, index, is_any_of(","));
                        vector<int> indexRowArr;
                        for(auto & it : valStrArr)
                            bs.length.push_back(atoi(it.c_str()));
                    }
                    valIndex = valIndex->next;
                }
            }
            nodeChild = nodeChild->next;
        }
    }
}

void getBlockingStages(vector<BLOCKING_STAGE>& theBlockingStages)
{
    xmlDoc *doc 				= xmlReadFile(configFileStr.c_str(), nullptr, 0);
    xmlXPathContext *pathCtx 	= xmlXPathNewContext(doc);
    xmlXPathObject *pathObj 	= xmlXPathEvalExpression((xmlChar *)"/rla-config/blocking-stages", pathCtx);

    BLOCKING_STAGE bs;

    for (int i = 0; i < pathObj->nodesetval->nodeNr; ++i) {
        xmlNode *nodeCurr = pathObj->nodesetval->nodeTab[i];
        xmlNode *nodeChild = nodeCurr->children;

        while (nodeChild) {
            if (!xmlStrcmp(nodeChild->name, (xmlChar *) "kmer-blocking")) {
                xmlXPathObject *pObj 	= xmlXPathEvalExpression(xmlGetNodePath(nodeChild), pathCtx);
                bs = BLOCKING_STAGE();
                getKmerBlockingParameters(pObj, bs);
                theBlockingStages.push_back(bs);
            } else if (!xmlStrcmp(nodeChild->name, (xmlChar *) "multi-blocking")) {
                xmlXPathObject *pObj 	= xmlXPathEvalExpression(xmlGetNodePath(nodeChild), pathCtx);
                bs = BLOCKING_STAGE();
                getMultiBlockingParameters(pObj, bs);
                theBlockingStages.push_back(bs);
            } else if (!xmlStrcmp(nodeChild->name, (xmlChar *) "multi-blocking-2")) {
                xmlXPathObject *pObj 	= xmlXPathEvalExpression(xmlGetNodePath(nodeChild), pathCtx);
                bs = BLOCKING_STAGE();
                getMultiBlockingParameters(pObj, bs);
                bs.blockingType = BLOCKING_TYPE_MULTI_2;
                theBlockingStages.push_back(bs);
            } else if (!xmlStrcmp(nodeChild->name, (xmlChar *) "soundex-blocking")) {
                xmlXPathObject *pObj 	= xmlXPathEvalExpression(xmlGetNodePath(nodeChild), pathCtx);
                bs = BLOCKING_STAGE();
                getMultiBlockingParameters(pObj, bs);
                bs.blockingType = BLOCKING_TYPE_SOUNDEX;
                theBlockingStages.push_back(bs);
            } else if (!xmlStrcmp(nodeChild->name, (xmlChar *) "doublemetaphone-blocking")) {
                xmlXPathObject *pObj    = xmlXPathEvalExpression(xmlGetNodePath(nodeChild), pathCtx);
                bs = BLOCKING_STAGE();
                getMultiBlockingParameters(pObj, bs);
                bs.blockingType = BLOCKING_TYPE_DOUBLEMETAPHONE;
                theBlockingStages.push_back(bs);
            }
            nodeChild = nodeChild->next;
        }
    }
    xmlXPathFreeContext(pathCtx);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

// read priority field index from config xml
vector<int> getPriorityField()
{
	xmlDoc *doc 				= xmlReadFile(configFileStr.c_str(), NULL, 0);
	xmlXPathContext *pathCtx 	= xmlXPathNewContext(doc);
	xmlXPathObject *pathObj 	= xmlXPathEvalExpression((xmlChar *)"/rla-config/version-config-param/priority", pathCtx);
	xmlXPathFreeContext(pathCtx);

	vector<int> priorityFieldArr;
	for (int i = 0; i < pathObj->nodesetval->nodeNr; ++i)
	{
		xmlNode *nodeCurr = pathObj->nodesetval->nodeTab[i];
		xmlNode *nodeChild = nodeCurr->children;

		while(nodeChild)
		{
			if(!xmlStrcmp(nodeChild->name, (xmlChar *)"value"))
			{
				string priority = (char *)xmlNodeListGetString(nodeCurr->doc, nodeChild->children, 1);
				vector<string> valStrArr;
				split(valStrArr, priority, is_any_of(","));
				for(vector<string>::iterator it = valStrArr.begin(); it != valStrArr.end(); ++it)
					priorityFieldArr.push_back(atoi((*it).c_str()));
				break;
			}
			nodeChild = nodeChild->next;
		}
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return priorityFieldArr;
}


// read comparison methods from config xml
vector<vector<vector<int> > > getInputComparisonPara()
{
	xmlDoc *doc 				= xmlReadFile(configFileStr.c_str(), nullptr, 0);
	xmlXPathContext *pathCtx 	= xmlXPathNewContext(doc);
	xmlXPathObject *pathObj 	= xmlXPathEvalExpression((xmlChar *)"/rla-config/version-config-param/comparison", pathCtx);
	xmlXPathFreeContext(pathCtx);

	xmlChar *attrVal;
	vector<int> attrSingleArr;
	vector<vector<int> > attrTypeArr;
	vector<vector<vector<int> > > attrArr(7);
	for (unsigned int i = 0; i < pathObj->nodesetval->nodeNr; ++i)
	{
		attrSingleArr.clear();
		xmlNode *nodeCurr	= pathObj->nodesetval->nodeTab[i];
		xmlNode *nodeChild	= nodeCurr->children;
		while(nodeChild)
		{
			if(!xmlStrcmp(nodeChild->name, (xmlChar *)"dist_calc_method"))
			{
				xmlNode *nodeGChild	= nodeChild->children;
				while(nodeGChild)
				{
					if(!xmlStrcmp(nodeGChild->name, (xmlChar *)"value"))
					{
						attrVal		= xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1);
						break;
					}
					nodeGChild	= nodeGChild->next;
				}
			}
			if(!xmlStrcmp(nodeChild->name, (xmlChar *)"comparing_attribute_indices"))
			{
				xmlNode *nodeGChild	= nodeChild->children;
				while(nodeGChild)
				{
					if(!xmlStrcmp(nodeGChild->name, (xmlChar *)"value"))
					{
						if(atoi((char *)attrVal) == 1) // edit distance
						{
							attrSingleArr.push_back(atoi((char *)xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1))); // indices
                            attrSingleArr.push_back(0);
                            attrArr[0].push_back(attrSingleArr);
						}
						else if(atoi((char *)attrVal) == 2) // reversal distance
						{
							string revStr	= string((char *)xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1));
							revStr.erase(remove(revStr.begin(), revStr.end(), ' '), revStr.end());
							vector<string> valStrArr;
							split(valStrArr, revStr, is_any_of(","));
							for (unsigned int t = 0; t < valStrArr.size(); ++t)
								attrSingleArr.push_back(atoi(valStrArr[t].c_str()));
							attrArr[1].push_back(attrSingleArr);
						}
						else if(atoi((char *)attrVal) == 3) // truncation distance
						{
							attrSingleArr.push_back(atoi((char *)xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1))); // indices

						}
						else if (atoi((char *)attrVal) == 5) // Q_gram distance  
						{
							attrSingleArr.push_back(atoi((char *)xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1))); // indices
							attrArr[5].push_back(attrSingleArr);

						}
						else if (atoi((char *)attrVal) == 6) // Hausdorff distance
						{
							cout << "Value - Distance Metric" << xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1) << endl;
							attrSingleArr.push_back(atoi((char *)xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1))); // indices
							attrArr[6].push_back(attrSingleArr);
						}
						break;
					}
					nodeGChild	= nodeGChild->next;
				}
			}
			if(!xmlStrcmp(nodeChild->name, (xmlChar *)"truncate_count"))
			{
				xmlNode *nodeGChild	= nodeChild->children;
				while(nodeGChild)
				{
					if(!xmlStrcmp(nodeGChild->name, (xmlChar *)"value"))
					{
						if(atoi((char *)attrVal) == 3) // truncation distance
						{
							attrSingleArr.push_back(atoi((char *)xmlNodeListGetString(nodeCurr->doc, nodeGChild->children, 1))); // truncation count
							attrArr[2].push_back(attrSingleArr);
							break;
						}
					}
					nodeGChild	= nodeGChild->next;
				}
			}

      if(!xmlStrcmp(nodeChild->name, (xmlChar *)"threshold"))
      {
        xmlNode *nodeGChild	= nodeChild->children;
        while(nodeGChild)
        {
          if(!xmlStrcmp(nodeGChild->name, (xmlChar *)"value"))
          {
            if(atoi((char *)attrVal) == 1) // edit distance threshold
            {
                attrSingleArr.clear();
                attrSingleArr.push_back(attrArr[0].back()[0]);
                attrSingleArr.push_back(atoi((char *)xmlNodeListGetString(nodeCurr->doc,
                                                                          nodeGChild->children,
                                                                          1))); // local threshold value
                attrArr[0].pop_back();
                attrArr[0].push_back(attrSingleArr);
              break;
            }

          }
          nodeGChild	= nodeGChild->next;
        }
      }
      nodeChild	= nodeChild->next;
		}
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return attrArr;
}


string getOutputDir()
{
	xmlDoc *doc;
	xmlNode *nodeCurr, *nodeChild, *nodeRoot;
	xmlChar *attrVal;
	xmlXPathContext *xpathCtx;
	xmlXPathObject *xpathObj;

	doc	= xmlReadFile(configFileStr.c_str(), nullptr, 0);

	nodeRoot	= xmlDocGetRootElement(doc);
	xpathCtx	= xmlXPathNewContext(doc);
	xpathObj	= xmlXPathEvalExpression((xmlChar *)"/rla-config/version-config-param/output_function/output_filename", xpathCtx);
	xmlXPathFreeContext(xpathCtx);

	attrVal		= (xmlChar *)"";
	nodeCurr	= xpathObj->nodesetval->nodeTab[0];
	nodeChild	= nodeCurr->children;
	while(nodeChild)
	{
		if(!xmlStrcmp(nodeChild->name, (xmlChar *)"value"))
		{
			attrVal	= xmlNodeListGetString(nodeCurr->doc, nodeChild->children, 1);
			break;
		}
		nodeChild	= nodeChild->next;
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return (char *)attrVal;
}


string getResultsLogFile()
{
    xmlDoc *doc;
    xmlNode *nodeCurr, *nodeChild, *nodeRoot;
    xmlChar *attrVal;
    xmlXPathContext *xpathCtx;
    xmlXPathObject *xpathObj;
    doc	= xmlReadFile(configFileStr.c_str(), nullptr, 0);
    nodeRoot	= xmlDocGetRootElement(doc);
    xpathCtx	= xmlXPathNewContext(doc);
    xpathObj	= xmlXPathEvalExpression((xmlChar *)"/rla-config/version-config-param/results_logging/filename",
                                         xpathCtx);
    xmlXPathFreeContext(xpathCtx);
    attrVal		= (xmlChar *)"";
    if(xpathObj->nodesetval->nodeTab == nullptr)
        return (char *)attrVal;
    nodeCurr	= xpathObj->nodesetval->nodeTab[0];
    nodeChild	= nodeCurr->children;
    while(nodeChild)
    {
        if(!xmlStrcmp(nodeChild->name, (xmlChar *)"value"))
        {
            attrVal	= xmlNodeListGetString(nodeCurr->doc, nodeChild->children, 1);
            break;
        }
        nodeChild	= nodeChild->next;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return (char *)attrVal;
}


/// Read data from input files
void readDataFromFile(vector<string>& fileNameArr)
{
	// store total attributes of each data set
	vector<int> fieldSizeArr;
	unsigned int k;
	for (unsigned int i = 0; i < fileNameArr.size(); ++i)
	{
		k = 0;
		for (unsigned int j = 0; j < indexDatasetArr.at(i).size(); ++j)
		{
			if (indexDatasetArr.at(i).at(j) < 0)
				continue;
			++k;
		}
		fieldSizeArr.push_back(k);
	}

	// read data
	unsigned int count;
    int recordPerFile = INT_MAX;
    if(isLimitRecordsEnabled) {
        recordPerFile = ceil(recordTotalCMD / fileNameArr.size());
        BOOST_LOG_TRIVIAL(warning) << "Reading only the first " << recordPerFile
                                    << " records of each file";
    }
	for (unsigned int i = 0; i < fileNameArr.size(); ++i)
	{
		recordStartIndArr.push_back(recordArr.size());
		count = 0;
        BOOST_LOG_TRIVIAL(info) << "Reading dataset file          : "<< fileNameArr[i];
		ifstream inFile;
		inFile.open(fileNameArr[i].c_str(), ifstream::in);
        if ( (inFile.rdstate() & std::ifstream::failbit ) != 0 ) {
            BOOST_LOG_TRIVIAL(error) << "Failed to open file: '" << fileNameArr[i].c_str() << "'";
            BOOST_LOG_TRIVIAL(error) << "Please correct the dataset path in the configuration file.";
            std::exit(EXIT_FAILURE);
        }
        string lineStr;
		vector<string> rowStrArr;
		if (inFile.good())
		{
			while (getline(inFile, lineStr))
			{
				if(lineStr.length() >= LINE_LENGTH_MIN)
				{
					rowStrArr.clear();
					vector<string> tempStrArr;
					split(tempStrArr, lineStr, is_any_of(","));
					for (unsigned int t = 0; t < tempStrArr.size(); ++t)
					{
						tempStrArr[t].erase(std::remove_if(tempStrArr[t].begin(), tempStrArr[t].end(), ::isspace), tempStrArr[t].end());
                        transform(tempStrArr[t].begin(), tempStrArr[t].end(), tempStrArr[t].begin(), ::tolower);
                        rowStrArr.push_back(tempStrArr[t]);
					}
					for (unsigned int j = rowStrArr.size(); j < fieldSizeArr[i]; ++j)
						rowStrArr.push_back("");
					rowStrArr.push_back(lexical_cast<string>(i));
					recordArr.push_back(rowStrArr);
                    ++count;
					if (isLimitRecordsEnabled && count >= recordPerFile)
						break;
				}
			}
		}
		inFile.close();
	}
	recordTotal = recordArr.size();
	if (distance_function == 0) {
		for (int i = 1; i < TYPE_TOTAL; i++){
			if (attrArr[i].size() > 0) {
				distance_function = i;
			}
		}
	}

	// concat common attributes for sorting
	string strSample(50, '0');
	int attrCommonTotal = attrArr[distance_function].size();
	lenMax		= 0;
	int lenMin = 150;
	strDataArr.resize(recordTotal);
	int totalLength = 0;
	for (int i = 0; i < recordTotal; ++i) {
		string s;
		for (int j = 0; j < attrCommonTotal; ++j)
			s += recordArr[i][attrArr[distance_function][j][0]] + "#";
		strDataArr[i]	= make_pair(i, s);
		totalLength += s.length();
		if(s.length() > lenMax)
			lenMax		= s.length();
		if (s.length() < lenMin)
			lenMin = s.length();
	}
	for (int i = 0; i < recordTotal; ++i) {
		int lenDiff		= lenMax - strDataArr[i].second.length();
		if(lenDiff > 0)
			strDataArr[i].second	+= strSample.substr(0, lenDiff);
	}
}


void findExactClusterEach()
{
	for (int i = 0; i < recordTotal; ++i)
	{
		vector<int> rowArr;
		rowArr.push_back(i);
		clusterExactIndArr.push_back(rowArr);
	}
}


/// Sort (radix sort) records using some attributes
void sortData() {
	vector<pair<int, string>> tempArr(recordTotal);
	for (int i = lenMax - 1; i >= 0; --i) {
		vector<int> countArr(256, 0);
		for (int j = 0; j < recordTotal; ++j) {
			assert(strDataArr[j].second.length() == lenMax);
			countArr[(strDataArr[j].second)[i]]++;
		}
		for (int k = 1; k < 256; ++k)
			countArr[k]	+= countArr[k - 1];
		for (int j = recordTotal - 1; j >= 0; --j)
			tempArr[--countArr[(strDataArr[j].second)[i]]]	= strDataArr[j];
		for (int j = 0; j < recordTotal; ++j)
			strDataArr[j]	= tempArr[j];
	}
}


/// Find clusters with no errors by grouping records using exact match
void findExactClusterSL() {
	sortData(); // sort records using some attributes as values (radix sort)
//    save_sorted_records(strDataArr, recordArr, "output/sorted_records");
    vector<int> clusterRowArr;
	clusterRowArr.push_back(strDataArr[0].first);
	for (int i = 1; i < recordTotal; ++i) {
		if(strDataArr[i].second.compare(strDataArr[i - 1].second) == 0)
			clusterRowArr.push_back(strDataArr[i].first);
		else {
			clusterExactIndArr.push_back(clusterRowArr);
			clusterRowArr.clear();
			clusterRowArr.push_back(strDataArr[i].first);
		}
	}
	clusterExactIndArr.push_back(clusterRowArr);
}


/// Clustering the data
void clusterData(LinkParam& lp)
{
    BOOST_LOG_TRIVIAL(info) << "Clustering started .......";
	startT			= clock();
	// find exact clusters
	clock_t currTS1	= clock();
	//findExactClusterEach();
	//findExactCluster(); // find clusters using exact match
	findExactClusterSL(); // find clusters using exact match
    BOOST_LOG_TRIVIAL(info) << "Number of exact clusters      : " << clusterExactIndArr.size();
	clusterExactT	= (double)(clock() - currTS1) / CLOCKS_PER_SEC;
	clock_t currTS2	= clock();
	vector<int> rootArr;
	findApproxCluster(rootArr); // find clusters using approximate match
	clusterApproxT	= (double)(clock() - currTS2) / CLOCKS_PER_SEC;
	// find all clusters using single linkage clustering
	clock_t currTS3	= clock();
	findFinalCluster(rootArr); // combine exact and approximate match
	clock_t clusterFinalT = (double)(clock() - currTS3) / CLOCKS_PER_SEC;
    clusterCombineT = (double)(clock() - currTS3) / CLOCKS_PER_SEC;
	clock_t currTS9		= clock();
	output(OUTPUT_SINGLE, lp);
	double totalOut1T	= (double)(clock() - currTS9) / CLOCKS_PER_SEC;
    if (!isCompleteLinkageEnabled) return;
    BOOST_LOG_TRIVIAL(info) << "Finding complete linkage ...";
	/// Find clusters using complete linkage clustering
	clock_t currTS4	= clock();
	for (unsigned int i = 0, m = clusterIndArr.size(); i < m; ++i) {
    if (clusterIndArr[i].size() > 1) {
      vector<Cluster> clusterTempArr;
      clusterGrp(clusterIndArr[i], clusterTempArr);
      for (unsigned int j = 0, n = clusterTempArr.size(); j < n; ++j)
        clusterArr.push_back(clusterTempArr[j]);
    } else if (clusterIndArr[i].size() == 1) {
      Cluster newC;
      newC.initCluster(0, recordArr[clusterIndArr[i][0]]);
      clusterArr.push_back(newC);}
    }
    clusterCLT = (double)(clock() - currTS4) / CLOCKS_PER_SEC;
    BOOST_LOG_TRIVIAL(info) << "Final Clustering Done!";
    BOOST_LOG_TRIVIAL(info) << "Number of clusters   :  " << clusterArr.size();
	clock_t currTS5	= clock();
	output(OUTPUT_FINAL, lp); // write output to a file
	clock_t outT = (double)(clock() - currTS5) / CLOCKS_PER_SEC;
    totalT	= (double)(clock() - startT) / CLOCKS_PER_SEC;
    BOOST_LOG_TRIVIAL(info) << "Clustering done";
    BOOST_LOG_TRIVIAL(info) << "Total Number of records                : " << recordTotal;
    BOOST_LOG_TRIVIAL(info) << "Reading time (s)                       : " << readT;
    BOOST_LOG_TRIVIAL(info) << "Writing output time (s)                : " << outT;
    BOOST_LOG_TRIVIAL(info) << "Total time (s)                         : " << (totalT - totalOut1T);
    BOOST_LOG_TRIVIAL(info) << "Exact clustering time (s)              : " << clusterExactT;
    BOOST_LOG_TRIVIAL(info) << "Approximate clustering time (s)        : " << clusterApproxT;
    BOOST_LOG_TRIVIAL(info) << "Final clustering time (s)              : " << clusterFinalT;
    BOOST_LOG_TRIVIAL(info) << "Complete linkage clustering time (s)   : " << clusterCLT;
    lp.precision = 0.1;
    lp.recall = 0.1;
}


/// Find exact clusters
void findExactCluster()
{
	clusterExactPairArr.resize(recordTotal);
	for (unsigned int i = 0; i < recordTotal; ++i)
		clusterExactPairArr[i].push_back(i);
	// find pairs of datasets such that dataset j contains dataset i fully
	for (unsigned int i = 0, m = indexDatasetArr.size(); i < m; ++i)
	{
		for (unsigned int j = 0; j < m; ++j)
		{
			if (i == j)	continue;
			bool sameF	= true; // if both have same indices of attributes
			unsigned int k = 0, n = indexDatasetArr.at(i).size();
			for (; k < n; ++k)
			{
				if (indexDatasetArr[i][k] > -1)
				{
					if (indexDatasetArr[j][k] == -1)	break;
				}
				else if (indexDatasetArr[j][k] > -1)	sameF	= false;
			}
			if (k >= n && (!sameF || (sameF && i < j))) // if sameF is true, there are two occasions. we can take when i < j or i > j
				findExactClusterPart(i, j);
		}
	}
	vector<int> tempArr;
	for (auto & i : clusterExactPairArr)
	{
		if (i[0] >= 0)
			for (unsigned int j = 1, n = i.size(); j < n; ++j)
			{
				if ((i[0] - i[j]) != 0)
				{
					appendExactInd(i[j], tempArr);
					for (int k : tempArr)
						if (k != i[0])
							i.push_back(k);
				}
			}
	}
	for (auto & i : clusterExactPairArr)
	{
		if (i[0] >= 0)
		{
			if (i.size() > 1)
			{
				vector<int> rowArr;
				for (unsigned int j = 0, n = i.size(); j < n; ++j)
				{
					if (find(rowArr.begin(), rowArr.end(), i[j]) - rowArr.begin() >= rowArr.size())
						rowArr.push_back(i[j]);
				}
				clusterExactIndArr.push_back(rowArr);
			}
			else
				clusterExactIndArr.push_back(i);
		}
	}
}


void appendExactInd(int pos, vector<int>& tempArr)
{
	if (clusterExactPairArr[pos][0] < 0)
		return;
	for (unsigned int i = 1; i < clusterExactPairArr[pos].size(); ++i)
		tempArr.push_back(clusterExactPairArr[pos][i]);
	clusterExactPairArr[pos][0]	= -1;
}

/// Find exact clusters for each pair
void findExactClusterPart(int indSub, int indSup)
{
	int indField;
	vector<StrPacket> tempArr;
	vector<int> truncArr;
	for (unsigned int i = 0, m = weightArr.size(); i < m; ++i)
		truncArr.push_back(-1);
	for (auto & i : attrArr[TYPE_TRUNC])
		truncArr[i[0]] = i[1];
	/// string packet creation for sub data set
	auto indStartSub 	= recordStartIndArr[indSub];
	auto indLastSub	 	= (indSub + 1) < fileNameArr.size() ? recordStartIndArr[indSub + 1] - 1 : recordArr.size() - 1;
	for (int i = indStartSub; i <= indLastSub; ++i)
	{
		string strCC; // concatenated string
		for (unsigned int j = 0, m = indexDatasetArr[indSub].size(); j < m; ++j)
		{
			indField	= indexDatasetArr[indSub][j];
			if (indField >= 0 && weightArr[j] > 0)
			{
				if (truncArr[j] > -1 && recordArr[i][indField].length() >= (unsigned)truncArr[j])
					strCC.append(recordArr[i][indField].substr(0, truncArr[j])).append("#");
				else
					strCC.append(recordArr[i][indField]).append("#");
			}
		}
		tempArr.push_back((StrPacket){strCC, i});
	}
	/// String packet creation for super data set
	auto indStartSup 	= recordStartIndArr[indSup];
	auto indLastSup	 	= (indSup + 1) < fileNameArr.size()?  recordStartIndArr[indSup + 1] - 1 : recordArr.size() - 1;
	int indexSupArr[indexDatasetArr[indSub].size()];
	for (int j = 0; j < indexDatasetArr[indSub].size(); ++j)
	{
		if (indexDatasetArr[indSub][j] > -1)
			indexSupArr[j]	= indexDatasetArr[indSup][j];
		else
			indexSupArr[j]	= -1;
	}
	for (int i = indStartSup; i <= indLastSup; ++i)
	{
		string strCC; // concatenated string
		for (unsigned int j = 0, m = indexDatasetArr[indSup].size(); j < m; ++j)
		{
			indField	= indexSupArr[j];
			if (indField >= 0 && weightArr[j] > 0)
			{
				if (truncArr[j] > -1 && recordArr[i][indField].length() >= (unsigned)truncArr[j])
					strCC.append(recordArr[i][indField].substr(0, truncArr[j])).append("#");
				else
					strCC.append(recordArr[i][indField]).append("#");
			}
		}
		tempArr.push_back((StrPacket){strCC, i});
	}
	cout << indSub << " in " << indSup << " record " << tempArr.size() << endl;
	vector<StrPacket> strSortedArr	= sortMSD(tempArr);
	vector<int> clusterRowArr;
	clusterRowArr.push_back(strSortedArr[0].ind);
	for (unsigned int i = 1, m = strSortedArr.size(); i < m; ++i)
	{
		if (strSortedArr[i].str.length() > 5 && strSortedArr[i].str == strSortedArr[i - 1].str)
		{
			clusterRowArr.push_back(strSortedArr[i].ind);
		}
		else
		{
			if (clusterRowArr.size() > 1)
			{
				int idSup	= -1;
				for (int j : clusterRowArr)
					if (j < indStartSub || j > indLastSub)
					{
						idSup	= j;
						break;
					}
				if (idSup > -1)
				{
					for (int j : clusterRowArr)
						if (j >= indStartSub && j <= indLastSub)
							clusterExactPairArr[idSup].push_back(j);
				}
			}
			clusterRowArr.clear();
			clusterRowArr.push_back(strSortedArr[i].ind);
		}
	}
	if (clusterRowArr.size() > 1)
	{
		int idSup	= -1;
		for (int j : clusterRowArr)
			if (j < indStartSub || j > indLastSub)
			{
				idSup	= j;
				break;
			}
		if (idSup > -1)
		{
			for (int j : clusterRowArr)
			{
				if (j >= indStartSub && j <= indLastSub)
					clusterExactPairArr[idSup].push_back(j);
			}
		}
	}
}


/// Find approximate clusters
/// using approximate match by blocking, generating edge list followed by findig connected components.
void findApproxCluster(vector<int>& rootArr)
{
	clock_t currTS2	= clock();
	vector<int> coverSetArr;
	for (int i = 0; i < fileNameArr.size(); ++i)
		coverSetArr.push_back(0);
    // Process blocking stages
    int curr_blocking_stage = 1;
    for(auto blockingStage: blockingStages) {
        switch (blockingStage.blockingType) {
            case BLOCKING_TYPE_UNKNOWN:
                break;
            case BLOCKING_TYPE_KMER: {
                BOOST_LOG_TRIVIAL(info) << "K-mer blocking started";
                algorithm_version_signature += " K";
                for(int i = 0; i < blockingStage.index.size(); ++i) {
                    algorithm_version_signature +=
                            " I" + to_string(blockingStage.index[i]) +
                            "K" + to_string(blockingStage.kmer[i]) +
                            "T" + to_string(blockingStage.type[i]);
                }
                BOOST_LOG_TRIVIAL(info) << algorithm_version_signature;
                int count;
                for (int i = 0; i < blockingStage.index.size(); ++i) {
                    count = 0;
                    for (int j = 0; j < indexDatasetArr.size(); ++j)
                        if (indexDatasetArr[j][blockingStage.index[i]] >= 0) {
                            ++count;
                            coverSetArr[j] = 1;
                        }
                    if (count < 2)
                        continue;
                    vector<vector<int> > blockArr;
                    createBlock(blockingStage.index[i],
                                blockingStage.kmer[i],
                                blockingStage.type[i],
                                blockArr);
                    if (isBlocksLoggingEnabled) {
                        save_blocks(clusterExactIndArr, recordArr,
                                    blockArr, "kmer-blocks-"+ to_string(curr_blocking_stage++));
                    }
                    createClusterEdgeList(blockArr);
                    for (int j: coverSetArr)
                        if (j < 1)
                            break;
                }
                BOOST_LOG_TRIVIAL(info) << "K-mer blocking ended";
                break;
            }
            case BLOCKING_TYPE_MULTI: {
                BOOST_LOG_TRIVIAL(info) << "MULTI blocking started";
                algorithm_version_signature += " MB ";
                multi_blocking_scheme blocks;
                multi_blocking_parameters mbps;
                for(int i=0; i < blockingStage.index.size(); ++i) {
                    multi_blocking_paramT mbp{};
                    mbp.index = blockingStage.index[i];
                    mbp.length = blockingStage.length[i];
                    mbps.push_back(mbp);
                }
                algorithm_version_signature += to_string(mbps);
                create_multi_blocking_scheme(mbps,
                                             clusterExactIndArr,
                                             recordArr,
                                             indexDatasetArr,
                                             blocks,
                                             isBlocksLoggingEnabled);
                for (const auto &[rp, bs]: blocks) {
                    auto a = rp.first;
                    auto b = rp.second;
                    if (isLinkageOk(recordArr[clusterExactIndArr[rp.first][0]],
                                    recordArr[clusterExactIndArr[rp.second][0]])) {
                        vector<int> edge(2);
                        edge[0] = a, edge[1] = b;
                        edgeArr.push_back(edge);  // {a,b}
                    }
                }
                }
                BOOST_LOG_TRIVIAL(info) << "MULTI blocking ended";
                break;
            case BLOCKING_TYPE_MULTI_2: {
                BOOST_LOG_TRIVIAL(info) << "MULTI blocking 2 started";
                algorithm_version_signature += " MB2 ";
                multi_blocking_parameters mbps;
                for(int i=0; i < blockingStage.index.size(); ++i) {
                    multi_blocking_paramT mbp{};
                    mbp.index = blockingStage.index[i];
                    mbp.length = blockingStage.length[i];
                    mbps.push_back(mbp);
                }
                algorithm_version_signature += to_string(mbps);
                unordered_map<string, vector<record_index>> blockArr;
                create_multi_blocking_scheme(mbps,
                                             clusterExactIndArr,
                                             recordArr,
                                             indexDatasetArr,
                                             blockArr,
                                             isBlocksLoggingEnabled);
                for(const auto& [blk_key, set_records] : blockArr) {
                    for(auto it_1=set_records.begin(); it_1!=set_records.end(); ++it_1) {
                        for(auto it_2=next(it_1, 1); it_2 != set_records.end(); ++it_2) {
                            int a = *it_1;
                            int b = *it_2;
                            if (isLinkageOk(recordArr[clusterExactIndArr[a][0]], recordArr[clusterExactIndArr[b][0]]))
                            {
                                vector<int> edge(2);
                                edge[0]	= a, edge[1] = b;
                                edgeArr.push_back(edge);  // {a,b}
                            }
                        }
                    }
                }
                }
                BOOST_LOG_TRIVIAL(info) << "MULTI blocking 2 ended";
                break;
            case BLOCKING_TYPE_SOUNDEX: {
                BOOST_LOG_TRIVIAL(info) << "Soundex blocking started";
                algorithm_version_signature += " SDX ";
                multi_blocking_parameters mbps;
                for(int i=0; i < blockingStage.index.size(); ++i) {
                    multi_blocking_paramT mbp{};
                    mbp.index = blockingStage.index[i];
                    mbp.length = blockingStage.length[i];
                    mbps.push_back(mbp);
                }
                algorithm_version_signature += to_string(mbps);
                unordered_map<string, vector<record_index>> blockArr;
                create_soundex_blocking(mbps,
                                         clusterExactIndArr,
                                         recordArr,
                                         indexDatasetArr,
                                         blockArr,
                                         isBlocksLoggingEnabled);
                for(const auto& [blk_key, set_records] : blockArr) {
                    for(auto it_1=set_records.begin(); it_1!=set_records.end(); ++it_1) {
                        for(auto it_2=next(it_1, 1); it_2 != set_records.end(); ++it_2) {
                            int a = *it_1;
                            int b = *it_2;
                            if (isLinkageOk(recordArr[clusterExactIndArr[a][0]], recordArr[clusterExactIndArr[b][0]]))
                            {
                                vector<int> edge(2);
                                edge[0]	= a, edge[1] = b;
                                edgeArr.push_back(edge);  // {a,b}
                            }
                        }
                    }
                }
                }
                BOOST_LOG_TRIVIAL(info) << "Soundex blocking ended";
                break;
            case BLOCKING_TYPE_DOUBLEMETAPHONE: {
                BOOST_LOG_TRIVIAL(info) << "Doublemetaphone blocking started";
                algorithm_version_signature += " DM ";
                multi_blocking_parameters mbps;
                for(int i=0; i < blockingStage.index.size(); ++i) {
                    multi_blocking_paramT mbp{};
                    mbp.index = blockingStage.index[i];
                    mbp.length = blockingStage.length[i];
                    mbps.push_back(mbp);
                }
                algorithm_version_signature += to_string(mbps);
                unordered_map<string, vector<record_index>> blockArr;

				// Start timer for doublemetaphone blocking
				clock_t doubleMetaphoneStart = clock();
                create_doublemetaphone_blocking(mbps,
                                         clusterExactIndArr,
                                         recordArr,
                                         indexDatasetArr,
                                         blockArr,
                                         isBlocksLoggingEnabled);
				
				for(const auto& [blk_key, set_records] : blockArr) {
                    for(auto it_1=set_records.begin(); it_1!=set_records.end(); ++it_1) {
                        for(auto it_2=next(it_1, 1); it_2 != set_records.end(); ++it_2) {
                            int a = *it_1;
                            int b = *it_2;
                            if (isLinkageOk(recordArr[clusterExactIndArr[a][0]], recordArr[clusterExactIndArr[b][0]]))
                            {
                                vector<int> edge(2);
                                edge[0] = a, edge[1] = b;
                                edgeArr.push_back(edge);  // {a,b}
                            }
                        }
                    }
                }
				// End timer and log the time
				double doubleMetaphoneTime = (double)(clock() - doubleMetaphoneStart) / CLOCKS_PER_SEC;	
				BOOST_LOG_TRIVIAL(info) << "Doublemetaphone blocking time (s): " << setprecision(1) << fixed << doubleMetaphoneTime;
				}
                BOOST_LOG_TRIVIAL(info) << "Doublemetaphone blocking ended";
                break;
            default:
                BOOST_LOG_TRIVIAL(warning) << "Unrecognized blocking type";
        }
    }
    BOOST_LOG_TRIVIAL(info) << "Approximate clustering time   : " << (double)(clock() - currTS2) / CLOCKS_PER_SEC;
	uf::find_connected_components(clusterExactIndArr, edgeArr, rootArr);
}


void createBlock(int indBlockField, int kmer, int type, vector<vector<int>>& blockArr)
{
    string strFieldType; //{26, 10, 36, 256}
    switch(type) {
        case 0:
            strFieldType = "Alphabetic (0)";
            break;
        case 1:
            strFieldType = "Numeric (1)";
            break;
        case 2:
            strFieldType = "Alphanumeric (2)";
            break;
        case 3:
            strFieldType = "ASCII (3)";
            break;
        default:
            strFieldType = "Unrecognized field data type";
    }
    BOOST_LOG_TRIVIAL(info) << "   Index                      : " << indBlockField;
    BOOST_LOG_TRIVIAL(info) << "   Kmer                       : " << kmer;
    BOOST_LOG_TRIVIAL(info) << "   Type                       : " << strFieldType;
    int blockTotal 	= pow(ALPHABET_SIZE_LIST[type], kmer);
	string strSample;
	if (type == 0) // alphabet is english alphabet
		strSample	= "aaaaaaaaaa"; // enough amount of characters for empty string (here 10)
	else
		strSample	= "0000000000";
	blockArr.resize(blockTotal);
	vector<string> record;
	int indFieldDataset, blockID;
	string blockFieldStr;
	for (int i = 0; i < clusterExactIndArr.size(); ++i) {
        record = recordArr[clusterExactIndArr[i][0]];
        indFieldDataset = indexDatasetArr[atoi(record[record.size() - 1].c_str())][indBlockField];
        if (indFieldDataset < 0)
            continue;
        blockFieldStr = record[indFieldDataset];
        int strLen = blockFieldStr.length();
        if (strLen < kmer) {
            blockFieldStr = strSample.substr(0, kmer - strLen) + blockFieldStr;
            strLen = kmer;
        }
        vector<int> codeRecordArr;
        for (int j = 0; j < blockFieldStr.length(); ++j) {
            if (((type == 0) && !(((int) blockFieldStr[j] >= 97 && (int) blockFieldStr[j] <= 122)))
                || ((type == 1) && !(((int) blockFieldStr[j] >= 48 && (int) blockFieldStr[j] <= 57)))
                || ((type == 2) && (!((((int) blockFieldStr[j] >= 97 && (int) blockFieldStr[j] <= 122)) ||
                                      (((int) blockFieldStr[j] >= 48 && (int) blockFieldStr[j] <= 57)))))
                    ) {
                blockFieldStr = blockFieldStr.substr(0, j) + blockFieldStr.substr(j + 1);
                --j;
            } else
                codeRecordArr.push_back((int) blockFieldStr[j]);
        }
        blockID = 0;
        for (int j = 0; j < blockFieldStr.length() - kmer + 1; ++j) {
            blockID = 0;
            for (int k = 0; k < kmer; ++k)
                if (type == 0)
                    blockID += (codeRecordArr[j + k] - 97) * (int) pow(ALPHABET_SIZE_LIST[type], kmer - k - 1);
                else if (type == 1)
                    blockID += (codeRecordArr[j + k] - 48) * (int) pow(ALPHABET_SIZE_LIST[type], kmer - k - 1);
                else if (type == 2) {
                    if (codeRecordArr[j + k] >= 97)
                        blockID += (codeRecordArr[j + k] - 97) * (int) pow(ALPHABET_SIZE_LIST[type], kmer - k - 1);
                    else
                        blockID += (codeRecordArr[j + k] - 22) *
                                   (int) pow(ALPHABET_SIZE_LIST[type], kmer - k - 1); // 48 - 26
                }
            if (!(blockID < 0 || blockID >= blockTotal))
                blockArr[blockID].push_back(i);
        }
    }
    int blkCount = 0;
    for(auto b: blockArr) {
        if (!b.empty()) {
            ++blkCount;
        }
    }
    BOOST_LOG_TRIVIAL(info) << "Number of non-empty blocks    : " << blkCount;
    BOOST_LOG_TRIVIAL(info) << "Maximum number of blocks      : " << blockTotal;
}


void createClusterEdgeList(vector<vector<int>>& blockArr)
{
	int	blockTotal	= blockArr.size();
	for (int i = 0; i < blockTotal; ++i)
	{
		if (blockArr[i].size() > 0)
		{
			generateEdgeList(blockArr[i]);
		}
	}
}


/// Generate edge list within a block
void generateEdgeList(vector<int>& blockRowArr)
{
	int blockItemTotal	= blockRowArr.size();

	vector<vector<string> > dataArr(blockItemTotal); // to make cache-efficient, keep records in a row
  vector<string> dataRow;
	for (int i = 0; i < blockItemTotal; ++i) {
    dataRow = recordArr[clusterExactIndArr[blockRowArr[i]][0]];
    dataArr[i] = dataRow;
  }
	vector<int> vectorArr;
	vectorArr.resize(blockItemTotal, 0);

	int temp, posTemp;
	int n = 0;
	for (int i = 0; i < blockItemTotal; i++)
	{
		if (vectorArr[i] == 0)
		{
			vector<int> tempArr;
			tempArr.push_back(i);
			posTemp			= 0;
			vectorArr[i]	= (++n);
			while (posTemp < tempArr.size())
			{
				temp		= tempArr[posTemp++];
				for (int j = 0; j < blockItemTotal; j++)
				{
					if (vectorArr[j] == 0 && j != temp)
					{	
						if (isLinkageOk(dataArr[temp], dataArr[j]))
						{
							tempArr.push_back(j);
							vectorArr[j]	= n;
							vector<int> edge(2);
							edge[0]	= blockRowArr[temp], edge[1] = blockRowArr[j];
							edgeArr.push_back(edge);
						}
					}
				}
			}
		}
	}

	dataArr.clear();
}


/// Find final clusters by combining exact clusters and approximate clusters.
void findFinalCluster(vector<int>& rootArr)
{
    BOOST_LOG_TRIVIAL(info) << "Number of clusters            : " << rootArr.size();
    BOOST_LOG_TRIVIAL(info) << "Combining exact & approx. clusters ...";
	int indCluster;
	clusterIndArr.assign(rootArr.size(), vector<int>());
	for (unsigned int i = 0; i < rootArr.size(); ++i)
	{
		indCluster	= uf::find_root(i, rootArr);
		for (unsigned int j = 0; j < clusterExactIndArr[i].size(); ++j)
			clusterIndArr[indCluster].push_back(clusterExactIndArr[i][j]);
	}
	clusterExactIndArr.clear();
	rootArr.clear();
	int k = 0;
	for (unsigned int i = 0; i < clusterIndArr.size(); ++i)
	{
		if (clusterIndArr[i].size() > 0)
			k++;
	}
    BOOST_LOG_TRIVIAL(info) << "Number of clusters            : " << k;
}


void clusterGrp(vector<int>& recordIndSingleGrpArr, vector<Cluster>& clusterSingleArr)
{
	vector<vector<string> > recordGrpArr(recordIndSingleGrpArr.size());
	for (unsigned int i = 0; i < recordIndSingleGrpArr.size(); ++i)
	{
		recordGrpArr[i]	= recordArr[recordIndSingleGrpArr[i]];
	}
	int recordGrpTotal	= recordGrpArr.size();
	for (int i = 0; i < recordGrpTotal; i++)
	{
		Cluster cluster;
		cluster.initCluster(0, recordGrpArr[i]);
		clusterSingleArr.push_back(cluster);
	}
	vector<vector<int> > matArr, vecArr;
	generateMatrix(recordGrpArr, matArr);
	generateVector(matArr, vecArr);
	int distCluster, distTemp;
	int idCluster1, idCluster2, idMinCluster, idMaxCluster;
	while (matArr.size() >= 2)
	{
		distCluster	= vecArr[0][0];
		idCluster1	= (int) vecArr[0][1];
		idCluster2	= (int) vecArr[0][2];
		for (unsigned int i = 1; i < vecArr.size(); i++)
		{
			distTemp	= vecArr[i][0];
			if (distCluster > distTemp)
			{
				distCluster	= distTemp;
				idCluster1	= (int) vecArr[i][1];
				idCluster2	= (int) vecArr[i][2];
			}
		}
		if (distCluster <= threshold)
		{
			if (idCluster1 > idCluster2)
			{
				idMinCluster	= idCluster2;
				idMaxCluster	= idCluster1;
			}
			else
			{
				idMinCluster	= idCluster1;
				idMaxCluster	= idCluster2;
			}
			Cluster newC;
			newC.initCluster(distCluster, clusterSingleArr[idMinCluster], clusterSingleArr[idMaxCluster]);
			clusterSingleArr.erase(clusterSingleArr.begin() + idMaxCluster);
			clusterSingleArr.erase(clusterSingleArr.begin() + idMinCluster);
			clusterSingleArr.push_back(newC);
			updateMatVec(matArr, vecArr, idMinCluster, idMaxCluster);
		}
		else
			break;
	}
	vector<string> recordItem;
	int fileInd, k;
	if (clusterSingleArr.size() > 1)
		postprocessCluster(clusterSingleArr);
	for (unsigned int i = 0; i < clusterSingleArr.size(); ++i)
	{
		if (clusterSingleArr[i].itemArr.size() > 1)
			for (unsigned int j = 0; j < clusterSingleArr[i].itemArr.size(); ++j)
			{
				recordItem	= clusterSingleArr[i].itemArr[j];
				fileInd		= atoi(recordItem[recordItem.size() - 1].c_str());
				for (k = 0; k < priorityFieldArr.size(); ++k)
				{
					if ((indexDatasetArr[fileInd][priorityFieldArr[k]] >= 0) && (recordItem[indexDatasetArr[fileInd][priorityFieldArr[k]]].length() > 0))
						break;
				}
				if (k >= priorityFieldArr.size())
				{
					recordItem	= clusterSingleArr[i].itemArr[j];
					clusterSingleArr[i].itemArr.erase(clusterSingleArr[i].itemArr.begin() + j);
					Cluster newC;
					newC.initCluster(0, recordItem);
					clusterSingleArr.push_back(newC);
				}
			}
	}
}


// generate 2d matrix of distances among records in recordGrpArr
void generateMatrix(vector<vector<string>>& recordGrpArr, vector<vector<int> >& matArr)
{
	int recordGrpTotal;
	recordGrpTotal	= recordGrpArr.size();
	for (int i = 0; i < recordGrpTotal; i++)
	{
		vector<int> rowMatArr;
		for (int j = 0; j < i; j++)
		{
			rowMatArr.push_back(linkage(recordGrpArr[i], recordGrpArr[j]));
		}
		matArr.push_back(rowMatArr);
	}
}


/// Generate vector of entries having min distance for each index
void generateVector(vector<vector<int> >& matArr, vector<vector<int> >& vecArr)
{
	int distMin, indThis, indOther;
	for (unsigned int i = 0; i < matArr.size(); i++)
	{
		indThis = i;
		if (i != (matArr.size()-1))
		{
			distMin		= matArr[i+1][i];
			indOther	= i + 1;
		}
		else
		{
			distMin 	= matArr[i][0];
			indOther	= 0;
		}
		for (unsigned int j = 0; j < matArr.size(); j++)
		{
			if (i > j)
			{
				if (distMin > matArr[i][j])
				{
					distMin		= matArr[i][j];
					indOther	= j;
				}
			}
			else if (i < j)
			{
				if (distMin > matArr[j][i])
				{
					distMin		= matArr[j][i];
					indOther	= j;
				}
			}
		}
		vector<int> obj;
		obj.push_back(distMin);
		obj.push_back(indThis);
		obj.push_back(indOther);
		vecArr.push_back(obj);
	}
}


// update matArr and vecArr after merging two clusters
void updateMatVec(vector<vector<int> > &matArr, vector<vector<int> > &vecArr, int indCluster1, int indCluster2)
{
	int indMin, indMax;
	int dist1, dist2;
	vector<int> rowMatArr;
	indMin	= indCluster1;
	indMax	= indCluster2;
	for (unsigned int i = 0; i < matArr.size(); i++)
	{
		if(i != indCluster1 && i != indCluster2)
		{
			if (i > indCluster1)
				dist1	= matArr[i][indCluster1];
			else
				dist1	= matArr[indCluster1][i];
			if (i > indCluster2)
				dist2	= matArr[i][indCluster2];
			else
				dist2	= matArr[indCluster2][i];
			rowMatArr.push_back(max(dist1, dist2));
		}
	}

	//remove rows and cols from m
	matArr.erase(matArr.begin() + indMax);
	matArr.erase(matArr.begin() + indMin);

	for (unsigned int i = 0; i < matArr.size(); i++)
	{
		if (matArr[i].size() >= indMax)
			matArr[i].erase(matArr[i].begin() + indMax);
		if (matArr[i].size() >= indMin)
			matArr[i].erase(matArr[i].begin() + indMin);
	}

	//add new row for r1+r2 to m
	matArr.push_back(rowMatArr);

	//get min distance and min distance index for r1+r2
	if (rowMatArr.size() != 0)
	{
		int distMinNew;
		int idThisNew, idOtherNew, idTemp;
		distMinNew	= rowMatArr[0];
		idOtherNew	= 0;
		for (unsigned int i = 0; i < rowMatArr.size(); i++)
		{
			if (distMinNew > rowMatArr[i])
			{
				distMinNew	= rowMatArr[i];
				idOtherNew	= i;
			}
		}
		//remove r1 and r2 from v
		vecArr.erase(vecArr.begin() + indMax);
		vecArr.erase(vecArr.begin() + indMin);
		//add new info for r1+r2
		idThisNew	= vecArr.size(); //insert location for info of r1+r2
		vector<int> rowVec;
		rowVec.push_back(distMinNew);
		rowVec.push_back(idThisNew);
		rowVec.push_back(idOtherNew);
		for (unsigned int i = 0; i < vecArr.size(); i++)
		{
			idTemp	= vecArr[i][1];
			if (idTemp > indMax)
				vecArr[i][1]	= (idTemp - 2); //update v[i][1] since remove of r1 and r2
			else if (idTemp > indMin)
				vecArr[i][1]	= (idTemp - 1);
			idTemp	= (int) vecArr[i][2];
			if (idTemp == indCluster1 || idTemp == indCluster2)
			{
				vecArr[i][0]	= matArr[matArr.size() - 1][round(vecArr[i][1])];
				vecArr[i][2]	= idThisNew; // update nearest neighbor
			}
			else
			{
				if (idTemp > indMax)
					vecArr[i][2]	= (idTemp - 2); //update v[i][2] since remove of r1 and r2
				else if (idTemp > indMin)
					vecArr[i][2]	= (idTemp - 1);

			}
		}
		vecArr.push_back(rowVec);
	}
}


void postprocessCluster(vector<Cluster>& clusterSingleArr)
{
	int l, maxVal, maxInd;
	Cluster cluster1, cluster2;
	vector<string> record;
	vector<vector<int> > usedAttrArr(clusterSingleArr.size());
	vector<int> isInClusterArr;
	for (unsigned int i = 0; i < clusterSingleArr.size(); ++i)
	{
		for (unsigned int j = 0; j < weightArr.size(); ++j)
			usedAttrArr[i].push_back(0);
		cluster1	= clusterSingleArr[i];
		for (unsigned int j = 0; j < cluster1.itemArr.size(); ++j)
		{
			record	= cluster1.itemArr[j];
			l		= atoi(record[record.size() - 1].c_str());
			for (unsigned int k = 0; k < indexDatasetArr[l].size(); ++k)
			{
				if (indexDatasetArr[l][k] > -1)
					usedAttrArr[i][k]	= 1;
			}
		}
	}
	for (unsigned int i = 0; i < clusterSingleArr.size(); ++i)
	{
		for (unsigned int j = 0; j < clusterSingleArr[i].itemArr.size(); )
		{
			isInClusterArr.assign(clusterSingleArr.size(), 0);
			isInClusterArr[i]	= 1;
			record	= clusterSingleArr[i].itemArr[j];
			for (unsigned int k = 0; k < clusterSingleArr.size(); ++k)
			{
				if(i == k)
					continue;
				cluster2	= clusterSingleArr[k];
				if(isInCluster(cluster2, record)) // check if this record also is in cluster2
					isInClusterArr[k]	= 1;
			}
			vector<vector<int> > usedAttrThisArr;
			processUsedAttr(usedAttrArr, isInClusterArr, atoi(record[record.size() - 1].c_str()), usedAttrThisArr);
			int fileIndThis	= atoi(record[record.size() - 1].c_str());
			for (unsigned int l = 0; l < clusterSingleArr.size(); ++l)
			{
				if(isInClusterArr[l] > 0)
				{
					for (int t = 0; t < clusterSingleArr[l].itemArr.size(); ++t)
					{
						vector<string> recordTemp	= clusterSingleArr[l].itemArr[t];
						int fileIndTemp	= atoi(recordTemp[recordTemp.size() - 1].c_str());
						for (unsigned int k = 0; k < priorityFieldArr.size(); ++k)
						{
							if (indexDatasetArr[fileIndThis][priorityFieldArr[k]] >= 0 && indexDatasetArr[fileIndTemp][priorityFieldArr[k]] >= 0)
							{
								if (record[indexDatasetArr[fileIndThis][priorityFieldArr[k]]] == recordTemp[indexDatasetArr[fileIndTemp][priorityFieldArr[k]]])
									isInClusterArr[l]	+= (priorityFieldArr.size() - k) * 100;
								else
									isInClusterArr[l]	-= (priorityFieldArr.size() - k) * 100;
							}
						}
					}
				}
			}
			maxVal	= isInClusterArr[i];
			maxInd	= i;
			for (unsigned int k = 0; k < isInClusterArr.size(); ++k)
			{
				if ((i != k) && (isInClusterArr[k] >= maxVal))
				{
					maxVal	= isInClusterArr[k];
					maxInd	= k;
				}
			}
			if(maxInd != i)
			{
				record	= clusterSingleArr[i].itemArr[j];
				clusterSingleArr[i].itemArr.erase(clusterSingleArr[i].itemArr.begin() + j);
				clusterSingleArr[maxInd].itemArr.push_back(record);
			}
			else
				j++;
		}
	}
}


bool isInCluster(Cluster& cluster, vector<string>& record)
{
	unsigned int i;
	for (i = 0; i < cluster.itemArr.size(); ++i)
		if (!isLinkageOk(record, cluster.itemArr[i]))
			break;
	if (i < cluster.itemArr.size())
		return false;
	else
		return true;
}


void processUsedAttr(vector<vector<int> >& usedAttrArr, vector<int>& isInClusterArr, int indDataset, vector<vector<int> >& usedThisAttrArr)
{
	usedThisAttrArr.resize(usedAttrArr.size());
	for (unsigned int i = 0; i < usedAttrArr.size(); ++i)
	{
		usedThisAttrArr[i].assign(weightArr.size(), 0);
		if (isInClusterArr[i] != 0)
		{
			for (unsigned int j = 0; j < weightArr.size(); ++j)
			{
				if (indexDatasetArr[indDataset][j] >= 0)
					usedThisAttrArr[i][j]	= (usedAttrArr[i][j]);
			}
		}
	}
}


void output(int outputType, LinkParam& lp)
{
	string fileNameStr	= outDir;
	if (outputType == OUTPUT_FINAL)
		fileNameStr.append("OutFinal");
	else
		fileNameStr.append("OutSingle");
    BOOST_LOG_TRIVIAL(info) << "Writing linkage output to file: " << fileNameStr;
	unsigned int clusterInd;
	vector<string> recordRow;
	ofstream outFile;
	outFile.open(fileNameStr.c_str(), ofstream::out);
	clusterInd	= 0;
	if (outputType == OUTPUT_FINAL)
	{
		for (unsigned int i = 0; i < clusterArr.size(); ++i)
		{
			if (clusterArr[i].itemArr.size() <= 0)
				continue;
			vector<int> countArr, indArr;
			++clusterInd;
			outFile << "cluster " << clusterInd << ": " << endl;
			for (unsigned int j = 0; j < clusterArr[i].itemArr.size(); ++j)
			{
				recordRow	= clusterArr[i].itemArr[j];
				int recordID	= atoi(recordRow[0].c_str());
				int pos		= find(indArr.begin(), indArr.end(), recordID) - indArr.begin();
				if (pos < countArr.size())
					countArr[pos]++;
				else
				{
					indArr.push_back(recordID);
					countArr.push_back(1);
				}
				for (unsigned int k = 0; k < recordRow.size(); ++k)
				{
					outFile << recordRow[k] << "\t";
				}
				outFile << endl;
			}
			outFile << endl;
		}
	}
	else
	{
		for (unsigned int i = 0; i < clusterIndArr.size(); ++i)
		{
			if (clusterIndArr[i].size() <= 0)
				continue;
			vector<int> countArr, indArr;
			++clusterInd;
			outFile << "cluster " << clusterInd << ": " << endl;
			for (unsigned int j = 0; j < clusterIndArr[i].size(); ++j)
			{
				recordRow	= recordArr[clusterIndArr[i][j]];
				int recordID	= atoi(recordRow[0].c_str());
				int pos		= find(indArr.begin(), indArr.end(), recordID) - indArr.begin();
				if (pos < countArr.size())
					countArr[pos]++;
				else
				{
					indArr.push_back(recordID);
					countArr.push_back(1);
				}
				for (unsigned int k = 0; k < recordRow.size(); ++k)
				{
					outFile << recordRow[k] << "\t";
				}
				outFile << endl;
			}
			outFile << endl;
		}
	}
	outFile  << endl << "Total Cluster: " << clusterInd << endl;
	outFile.close();
    BOOST_LOG_TRIVIAL(info) << "Number of clusters:           : " << clusterInd;
}


bool isLinkageOk(vector<string>& a, vector<string>& b)
{
	double w	= calculateDistAll(a, b);

	/*
		The threshold and corresponding calculation is different for Q-GRAM distance
		If threshold < 1 && > 0 --> then the distance must be Q-GRAM and hence second condition needs to be evaluated
		For all other distances we need to evaluate first condition.
	*/

	if (threshold >= 1){
		return ( threshold - w >= 0 )? true : false;
	}
	else {
		return ( threshold - w <= 0 )? true : false;
	}
}


double linkage(vector<string>& a, vector<string>& b)
{
	double w = calculateDistAll(a, b);
	return w;
}


double calculateDistAll(vector<string>& a, vector<string>& b)
{
	double w	= 0;
	switch(distance_function){
		case 0:
			w	+= calculateEditDist(a, b, attrArr[TYPE_EDIT], threshold - w);
			break;
		case 1:
			w	+= calculateRevDist(a, b, attrArr[TYPE_REVERSAL], threshold - w);
			break;
		case 2:
			w	+= calculateTruncDist(a, b, attrArr[TYPE_TRUNC], threshold - w);
			break;
		case 5:
			w       += calculateQGramDist(a,b, attrArr[TYPE_QGRAM],14);
			break;
		case 6:
			w       += calculateHausdorffDist(a,b, attrArr[TYPE_HAUSDORFF],13);
			break;
		default:
			w	+= calculateEditDist(a, b, attrArr[TYPE_EDIT], threshold - w);
	}
	return w;
}


double calculateQGramDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt,int k)
{
	int ind, a_ind, b_ind;
	double temp, w;
	string s1, s2;
	w 	= 0;
	int al_ind = atoi(a[a.size() - 1].c_str()), bl_ind = atoi(b[b.size() - 1].c_str());
	for (int g = 0; g < compareAtt.size(); g++)
	{
		ind 	= compareAtt[g][0];
		a_ind	= indexDatasetArr[al_ind][ind];
		b_ind	= indexDatasetArr[bl_ind][ind];
		if (a_ind == -1 || b_ind == -1 || a[a_ind].length() == 0 || b[b_ind].length() == 0)
			continue;
		s1 		= a[a_ind];
		s2 		= b[b_ind];
		if (s1.size() >= k && s2.size() >= k ){
			temp = calculateBasicQgram(s1, s2,k) * weightArr[ind];
			w += temp;
		}
	}
	return w;
}


double calculateHausdorffDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt,int k)
{
	int ind, a_ind, b_ind;
	double temp, w;
	string s1, s2;
	w 	= 0;
	int al_ind = atoi(a[a.size() - 1].c_str()), bl_ind = atoi(b[b.size() - 1].c_str());
	for (int g = 0; g < compareAtt.size(); g++)
	{
		ind 	= compareAtt[g][0];
		a_ind	= indexDatasetArr[al_ind][ind];
		b_ind	= indexDatasetArr[bl_ind][ind];
		if (a_ind == -1 || b_ind == -1 || a[a_ind].length() == 0 || b[b_ind].length() == 0)
			continue;
		s1 		= a[a_ind];
		s2 		= b[b_ind];
		if (s1.size() >= k && s2.size() >= k ){
			temp = calculateBasicHausdorffDistance(s1, s2,k) * weightArr[ind];
			w += temp;
		}
	}
	return w;
}


int calculateEditDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt, int threshRem)
{
	int w, ind, temp, a_ind, b_ind;
    int thresholdAtt;
	w 	= 0;
	int al_ind = atoi(a[a.size() - 1].c_str()), bl_ind = atoi(b[b.size() - 1].c_str());
	for (int g = 0; g < compareAtt.size(); g++)
	{
		ind 	= compareAtt[g][0];
        thresholdAtt = compareAtt[g][1];
		a_ind	= indexDatasetArr[al_ind][ind];
		b_ind	= indexDatasetArr[bl_ind][ind];
		if (a_ind == -1 || b_ind == -1 || a[a_ind].length() == 0 || b[b_ind].length() == 0)
			continue;
		string s1 		= a[a_ind];
		string s2 		= b[b_ind];
		if (threshRem < 0)
            return threshold + 1;
        int dist 	= ed.edit_dist_A2(s1, s2, threshRem);
        if (thresholdAtt > 0 && dist > thresholdAtt) {
            return threshold + 1;
        }
        temp 	= dist * weightArr[ind];
		w 			+= temp;
		threshRem 	= threshRem-temp;
	}
	return w;
}


int calculateRevDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt, int threshRem)
{
	int i, w, w1, w2, ind, ind1, ind2;
	w	= 0;
	w1	= 0;
	w2	= 0;
	for(i = 0; i < compareAtt.size(); ++i)
	{
		ind		= compareAtt[i][0];
		ind1	= indexDatasetArr[atoi(a[a.size() - 1].c_str())][ind];
		ind2	= indexDatasetArr[atoi(b[b.size() - 1].c_str())][ind];
		if (ind1 == -1 || ind2 == -1 || a[ind1].length() == 0 || b[ind2].length() == 0)
			continue;
		string str1	= a[ind1];
		string str2	= b[ind2];
		w1		= ed.edit_dist_A2(str1, str2, threshRem - w);
		ind		= compareAtt[i][1];
		ind1	= indexDatasetArr[atoi(a[a.size() - 1].c_str())][ind];
		ind2	= indexDatasetArr[atoi(b[b.size() - 1].c_str())][ind];
		if (ind1 == -1 || ind2 == -1 || a[ind1].length() == 0 || b[ind2].length() == 0)
			continue;
		string  str12	= a[ind1];
		string  str22	= b[ind2];
		w1		= ed.edit_dist_A2(str12, str22, threshRem - w);
		ind		= compareAtt[i][0];
		ind1	= indexDatasetArr[atoi(a[a.size() - 1].c_str())][ind];
		ind		= compareAtt[i][1];
		ind2	= indexDatasetArr[atoi(b[b.size() - 1].c_str())][ind];
		if (ind1 == -1 || ind2 == -1 || a[ind1].length() == 0 || b[ind2].length() == 0)
			continue;
		string  str13	= a[ind1];
		string  str23	= b[ind2];
		w2		= ed.edit_dist_A2(str13, str23, threshRem - w);
		ind		= compareAtt[i][1];
		ind1	= indexDatasetArr[atoi(a[a.size() - 1].c_str())][ind];
		ind		= compareAtt[i][0];
		ind2	= indexDatasetArr[atoi(b[b.size() - 1].c_str())][ind];
		if (ind1 == -1 || ind2 == -1 || a[ind1].length() == 0 || b[ind2].length() == 0)
			continue;
		string  str14	= a[ind1];
		string  str24	= b[ind2];
		w2		+= ed.edit_dist_A2(str14, str24, threshRem - w);
		w		+= min(w1, w2);
	}
	return w;
}


int calculateTruncDist(vector<string>& a, vector<string>& b, vector<vector<int> >& compareAtt, int threshRem)
{
	int w, indA, indB, indSetA, indSetB, count, ind;
	string str1, str2;
	w		= 0;
	indSetA	= atoi(a[a.size() - 1].c_str());
	indSetB	= atoi(b[b.size() - 1].c_str());
	for (unsigned int i = 0; i < compareAtt.size(); ++i)
	{
		ind		= compareAtt[i][0];
		count	= compareAtt[i][1];
		indA	= indexDatasetArr[indSetA][ind];
		if (indA == -1 || a[indA].length() == 0)
			continue;
		indB	= indexDatasetArr[indSetB][ind];
		if (indB == -1 || b[indB].length() == 0)
			continue;
		str1	= a[indA];
		str2	= b[indB];
		if (str1.length() >= count && str2.length() >= count)
		{
			str1	= str1.substr(0, count);
			str2	= str2.substr(0, count);
		}
		else
		{
			int len	= min(str1.length(), str2.length());
			str1	= str1.substr(0, len);
			str2	= str2.substr(0, len);
		}
		w	+= ed.edit_dist_A2(str1, str2, threshRem - w);
	}
	return w;
}


set<string> generateKmers(string& str,int k)
{
	set<string> k_mer_list;
	for(int i = 0; i <= (str.length() - k);i++){
		std::string k_mer = str.substr(i,k);
		k_mer_list.insert(k_mer);
	}
	return k_mer_list;
}


int calculateHammingDistance(string& str1, string& str2){
	int hammingDistance = 0;
	for (int i = 0; i < str1.length(); i++){
		if (i > str2.length()){
		    break;
		}
		else
        {
		    if (str1[i] != str2[i]){
		    	hammingDistance += 1;
		    }
		}
	}
	return hammingDistance;
}


double calculateBasicHausdorffDistance(string& str1,string& str2,int threshRem)
{
	int k = threshRem;
	set<string> s1 = generateKmers(str1,k);
	set<string> s2 = generateKmers(str2,k);
	set<string>::iterator itr, itr2;
	vector<int> min_distances;
	for (itr = s1.begin(); itr != s1.end(); itr++)
	{
		vector<int> distance;
		for (itr2 = s2.begin(); itr2 != s2.end(); itr2++){
			std::string tmp1 = *itr;
			std::string tmp2 = *itr2;
			distance.push_back(calculateHammingDistance(tmp1,tmp2));
		}
		min_distances.push_back(*min_element(distance.begin(), distance.end()));
	}
	int size_difference = s1.size() - s2.size();
	if (size_difference < 0) {
		size_difference = - size_difference;
	}
	return (double) (*max_element(min_distances.begin(), min_distances.end()) + size_difference);
}


double calculateBasicQgram(string& str1, string& str2,int threshRem)
{
	int len_str1,len_str_2;
	int k = threshRem;
	set<string> s1 = generateKmers(str1,k);
	set<string> s2 = generateKmers(str2,k);
	set<string> intersect;
	set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                inserter(intersect, intersect.begin()));
	set<string> union_set;
	set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                 inserter(union_set, union_set.begin()));
	double q_gram_distance = (double)intersect.size() / (double)union_set.size();
	return q_gram_distance;
}
