/// @file edit_distance.h
/// @details header file for edit distance related code.
#ifndef RLA_CL_N_EDIT_DISTANCE_H
#define RLA_CL_N_EDIT_DISTANCE_H
#include <string>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <climits>
/// Maximum string size
#define MAX_STRING_LENGTH 50
#define MATRIX_SIZE (MAX_STRING_LENGTH + 1)
#define MATRIX_TYPE int8_t
#define large_number INT8_MAX
using namespace std;
class edit_distance{
private:
    /// shared edit distance 2D matrix
    MATRIX_TYPE matArr[MATRIX_SIZE][MATRIX_SIZE] = {0};
    MATRIX_TYPE ED[MATRIX_SIZE][MATRIX_SIZE] = {0};
    static ofstream init_log_file();
public:
    edit_distance();
    void log_edit_matrix(ofstream& ofs, string& str1, string& str2, MATRIX_TYPE matrix[MATRIX_SIZE][MATRIX_SIZE]);
    int edit_dist_q_log(string& str1, string& str2, int threshRem);
    int edit_dist_q(string& str1, string& str2, int threshRem);
    int edit_dist_q2(string& str1, string& str2, int threshRem);
    int edit_dist_J(string& str1, string& str2, int threshRem);
    int edit_dist_J2_log(string& str1, string& str2, int threshRem);
    int edit_dist_J2(string& str1, string& str2, int threshRem);
    int edit_dist_A(string& str1, string& str2, int threshRem);
    int edit_dist_A2(string& str1, string& str2, int threshRem);
    int edit_dist_A3(string& str1, string& str2, int threshRem);
    int edit_dist_Ukkonen(string& a, string& b, int threshRem=large_number);
};
#endif //RLA_CL_N_EDIT_DISTANCE_H
