/// @file ed_perf.cpp
/// @details This is a test driver program that measures the performance of various implementations of edit distance
/// computation functions


#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "RLA_CL.h"
#include <iomanip>
#include "edit_distance.h"

#define BUFFER_SIZE 100
#define NUMBER_OF_TEST_LOOPS 5

using namespace std;


/// @brief Print usage statement for the user
///
/// Display example usage.
void usage() {
  cout << "Usage:" << endl;
  cout << "  test_edit_distance FILE NUMBER_OF_STRING_PAIRS" << endl;
  cout << "For example:" << endl;
  cout << "      test_edit_distance edit_pairs.csv 1000000" << endl;
}


/// @brief Load string pairs from the give file
/// @param file_name file containing string pairs in csv format with no header line
/// @param str_pairs data place holder for string pairs being read
/// @param number_of_pairs number of lines or string pairs to be read
///
/// Opens the given csv file, read string pairs therein, and store them in the
/// `str_pairs` placeholder.
void load_string_pairs(string& file_name, vector<vector<string> >& str_pairs, long int number_of_pairs) {
  ifstream ifs;
  char buf[BUFFER_SIZE]; // buffer
  int line_number = 0;
  cout << "Opening file: " << file_name << " ... ";
  ifs.open(file_name, ifstream::in);
  if (ifs.fail()) {
    cout << "FAILED" << endl;
    cout << "error opening file: " << file_name << endl;
    exit(2);
  }
  cout << "OK" << endl;
  cout << "Reading " << number_of_pairs << " string pairs" << " ... ";
  while (ifs.good() && line_number < number_of_pairs) {
    for(int i=0; i<2; ++i) {
      ifs.getline(buf, BUFFER_SIZE, ',');
      str_pairs[line_number][i] = buf;
    }
    ifs.ignore(BUFFER_SIZE, '\n');
    line_number++;
  }
  cout << "DONE" << endl;
}
/// @brief Verify an edit distance algorithm
/// @param string_pairs to be used in verification
int verify_edit_distance_func(vector<vector<string> >& string_pairs) {
  edit_distance ed = edit_distance();
  size_t number_of_string_pairs = string_pairs.size();
  for (int threshRem = 0; threshRem < 12; ++threshRem) {
    for (int i = 0; i < number_of_string_pairs; ++i) {
      string& s1 = string_pairs[i][0];
      string& s2 = string_pairs[i][1];
      int J = ed.edit_dist_J(s1, s2, threshRem);
      int A3 = ed.edit_dist_A3(s1, s2, threshRem);
      if (J != A3) {
        cout << s1 << "," << s2 << " --J--> " << J << " --A3--> " << A3 << " th = " << threshRem << endl;
        return(-1);
      }
    }
  }
  return 0;
}
/// @brief Main program entry point
/// @param argv [1] input file name
/// @param argv [2] number of record pairs to read
int main(int argc, char** argv) {
  if (argc<3) {
    usage();
    exit(1);
  }
  string file_name = argv[1];
  long int number_of_string_pairs = strtol(argv[2], nullptr, 10);

  vector<vector<string> > string_pairs;
  string_pairs.resize(number_of_string_pairs);
  for(auto& a: string_pairs) {
    a.resize(2);
  }
  load_string_pairs(file_name, string_pairs, number_of_string_pairs);
  edit_distance ed=edit_distance();
  int th;
//  s1 = "johnny";  s2 = "jonathan"; th = 5;
//  s1 = "baker";  s2 = "barbour"; th = 2;
//  s1 = "kari"; s2 = "karen"; th = 2;
//  s1 = "deborah"; s2 = "debra"; th = 2;
//  s1 = "sarah"; s2 = "sandra"; th = 3;
//  s1 = "bowers"; s2 = "bookbinder"; th = 7;
//  s1 = "ty"; s2 = "tyrell"; th = 4;
//  s1 = "johnett"; s2 = "josephine"; th = 6;
//  s1 = "lexington";  s2 = "lewisville"; th = 1;
//  int dist = ed.edit_dist_A2(s1, s2, th);
//  cout << "EDIT(" << s1 << "," << s2 << ") = " << dist << endl;
  cout << "Verifying correctness of edit distance computation ... ";
  if (verify_edit_distance_func(string_pairs) == 0) {
    cout << " OK" << endl;
  } else {
    cout << " Verification Failed. " << endl;
    exit(1);
  }
//  cout << endl;
//  cout << "A3" << endl;
  vector<double> A2;
  for (int threshRem=0; threshRem<12; ++threshRem) {

    long int number_of_string_pairs_compared = 0;
    clock_t start= clock();
    for (int t=0; t<NUMBER_OF_TEST_LOOPS; ++t) {
      for (int i=0; i<number_of_string_pairs; ++i) {
        string& s1 = string_pairs[i][0];
        string& s2 = string_pairs[i][1];
        ed.edit_dist_A3(s1, s2, threshRem);
        ++number_of_string_pairs_compared;
      }
    }
//    cout << "Threshold : " << threshRem << endl;
    double time_taken = (double)(clock() - start) / CLOCKS_PER_SEC;
//    cout << "Computed " << number_of_string_pairs_compared << " edit distances in " << time_taken << " seconds" << endl;
    double speed = (double) number_of_string_pairs_compared / 1000000.0 / time_taken;
//    cout << "Speed : " << speed << " MEPS (Million Edit Distances per Seconds)" << endl;
//    cout << threshRem << ",";
//      cout << speed << endl;
      A2.push_back(speed);
  }
//  cout << endl;
//  cout << "UKKonen" << endl;
  vector<double> A3;
  for (int threshRem=0; threshRem<12; ++threshRem) {

    long int number_of_string_pairs_compared = 0;
    clock_t start= clock();
    for (int t=0; t<NUMBER_OF_TEST_LOOPS; ++t) {
      for (int i=0; i<number_of_string_pairs; ++i) {
        string& s1 = string_pairs[i][0];
        string& s2 = string_pairs[i][1];
        ed.edit_dist_Ukkonen(s1, s2, threshRem);
        ++number_of_string_pairs_compared;
      }
    }
//    cout << "Threshold : " << threshRem << endl;
    double time_taken = (double)(clock() - start) / CLOCKS_PER_SEC;
//    cout << "Computed " << number_of_string_pairs_compared << " edit distances in " << time_taken << " seconds" << endl;
    double speed = (double) number_of_string_pairs_compared / 1000000.0 / time_taken;
//    cout << "Speed : " << speed << " MEPS (Million Edit Distances per Seconds)" << endl;
//    cout << threshRem << ",";
//    cout << speed << endl;
    A3.push_back(speed);
  }
  cout << endl;
  cout << "A3,Ukkonen" << endl;
  cout << fixed;
  for(int i=0; i<A2.size(); ++i) {
    cout << setprecision(1) << A2[i] << "," << setprecision(1) << A3[i] << endl;
  }

//  cout << endl;
//  cout << "Ukkonen-A3" << endl;
//  cout << fixed;
//  for(int i=0; i<A2.size(); ++i) {
//    cout << setprecision(1) << A3[i]-A2[i] << endl;
//  }

}
