/// @file edit_distance.cpp
/// @details implementation file for edit distance related code.
#include "edit_distance.h"

edit_distance::edit_distance(){
  for(int i=0; i<MATRIX_SIZE; ++i) {
    ED[0][i]=(MATRIX_TYPE)i;
    ED[i][0]=(MATRIX_TYPE)i;
  }
}

/// @brief Quadratic edit distance computation algorithm
/// @param str1 first string
/// @param str2 second string
/// @param threshRem threshold remaining or bound on the edit distance to be returned
/// @return edit distance calculated.
/// @Description If the edit distance is greater than given threshold, return @c large_number
/// @note returned value is not always correct.
int edit_distance::edit_dist_q(string& str1, string& str2, int threshRem)
{
  int row, col, i, j;

  row	= (int) str1.length() + 1;
  col 	= (int) str2.length() + 1;

  for(i = 0; i < row; i++)
  {
    for(j = 0; j < col; j++)
    {
      if (i == 0)
        matArr[i][j] = (MATRIX_TYPE)j;
      else if (j == 0)
        matArr[i][j] = (MATRIX_TYPE)i;
      else
      {
        int e = ((int) str1[i - 1] == (int) str2[j - 1]) ? 0 : 1;
        matArr[i][j] = min(min((MATRIX_TYPE)(matArr[i - 1][j - 1]+e),
                               (MATRIX_TYPE)(matArr[i - 1][j] + 1)),
                           (MATRIX_TYPE) (matArr[i][j - 1] + 1));
      }

      if((row - col) == (i - j) && (matArr[i][j] > threshRem))
      {
        return large_number;
      }
    }
  }
  return (matArr[row - 1][col - 1]);
}
/// @brief optimized edit distance computation algorithm
/// @authors Joyanta, Mamun, and Ahmed
/// @param str1 first string
/// @param str2 second string
/// threshRem threshold remaining or bound on the edit distance to be returned
/// @return edit distance calculated.
/// @Description Given two strings and a threshold value as input, the edit distance
/// between the two strings is computed and if the distance exceeds @c threshRem,
/// the function returns @c large_number
/// @Note Only cells @kp = (ceil(k+q)/2.0) off the main diagonal need to be computed
int edit_distance::edit_dist_A(string& str1, string& str2, int threshRem)
{
  int &k = threshRem;
  int n = (int) str1.length();
  int m = (int) str2.length();

  bool swapped = false;

  if (n>m) {  // ensure m >= n
    swap(m,n);
    swapped = true;
  }

  int q = m-n;
  int kp; // k prime
  kp = (k<2) ? k : (int) ceil((k + q)/2.0);
  kp = max(kp, q);
  kp = min(kp, k);

  if ((q > k))
    return large_number;
  if(str1==str2)
    return 0;
  if (k == 0)
    return large_number;
  else if((2 * kp + 1) >= m)
    return edit_dist_q(str1, str2, k);
  else
  {
    string s1 = str1;
    string s2 = str2;
    int row, col, diagonal;
    int i, j;

    if (swapped)
    {
      swap(s1,s2);
    }

    row	 		= n + 1;
    col 		= kp + kp + 1;
    diagonal 	= kp + q;

    for(i = 0; i < kp + 1; i++)
    {
      for(j = kp - i; j < col; j++)
      {
        if (i == 0)
          matArr[i][j]	= (MATRIX_TYPE)(j - kp);
        else if(j == (kp - i))
          matArr[i][j] 	= (MATRIX_TYPE) (matArr[i - 1][j + 1] + 1);
        else {
          int e = ((int)s1[i - 1] == (int)s2[j - (kp - i) - 1]) ? 0 : 1;
          if(j != (col - 1))
            matArr[i][j]	= min(min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                      (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1)),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
          else
            matArr[i][j]	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }

        if((j == diagonal) && matArr[i][j] > k)
          return large_number;
      }
    }

    for(i = kp + 1; i < m - kp + 1; i++)
    {
      for(j = 0; j < col; j++)
      {
        int e = ((int)s1[i - 1] == (int)s2[j + (i - kp) - 1]) ? 0 : 1;
        if(j == 0)
        {
            matArr[i][j]	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1));
        }
        else if(j != (col - 1))
        {
            matArr[i][j] 	= min(min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                      (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1)),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }
        else
        {
            matArr[i][j] 	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }
        if((j == diagonal) && (matArr[i][j] > k))
          return large_number;
      }
    }

    for(i = m - kp + 1; i < row; i++)
    {
      for(j = 0; j < col - i + m - kp; j++)
      {
        int e = ((int)s1[i - 1] == (int)s2[j + (i - kp) - 1]) ? 0 : 1;
        if(j == 0)
        {
            matArr[i][j]	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1));
        }
        else
        {

            matArr[i][j] 	= min(min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                      (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1)),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }
        if((j == diagonal) && (matArr[i][j] > k))
          return large_number;
      }
    }
    return matArr[row - 1][diagonal];
  }
}


/// @brief Quadratic edit distance computation algorithm with logging
/// @param str1 first string
/// @param str2 second string
/// @param threshRem threshold remaining or bound on the edit distance to be returned
/// @return edit distance calculated.
/// @Description If the edit distance is greater than given threshold, return @c large_number
/// @note returned value is not always correct.
int edit_distance::edit_dist_q_log(string& str1, string& str2, int threshRem)
{
  int row, col, i, j;

  static ofstream ed_log = init_log_file();
  int edit_dist; /// edit distance return value

  row	= (int) str1.length() + 1;
  col 	= (int) str2.length() + 1;

  for(i = 0; i < row; i++)
  {
    for(j = 0; j < col; j++)
    {
      if (i == 0)
        matArr[i][j] = (MATRIX_TYPE)j;
      else if (j == 0)
        matArr[i][j] = (MATRIX_TYPE)i;
      else
      {
        int e = ((int)str1[i-1] == (int)str2[j-1]) ? 0 : 1;
        matArr[i][j] = min(min((MATRIX_TYPE)(matArr[i - 1][j - 1] + e),
                               (MATRIX_TYPE)(matArr[i - 1][j] + 1)),
                           (MATRIX_TYPE)(matArr[i][j - 1] + 1));
      }

      if((row - col) == (i - j) && (matArr[i][j] > threshRem))
      {
        edit_dist = large_number;
        goto calculateBasicED2_ret;
      }
    }
  }
  edit_dist = matArr[row - 1][col - 1];
  i = row - 1;
  j = col - 1;
  calculateBasicED2_ret:
  static int cnt = 0;
  if (cnt<10)
  {
    ed_log << "Threshold = " << threshRem << endl;
    log_edit_matrix(ed_log, str1, str2, matArr);
    ed_log << "(i,j) = (" << i << "," << j << ")" << endl;
    ed_log << "edit distance = " << edit_dist << endl;
    ed_log << endl;
    cnt++;
  }
  else
  {
    ed_log.close();
  }

  return (edit_dist);
}
/// @brief initialize edit distance log file
/// @returns edit distance log file stream
ofstream edit_distance::init_log_file() {
  string f_name = "output/edit_distances.log";
  ofstream ofs;
  ofs.open(f_name.c_str(), ofstream::out);
  return ofs;
}
/// @Description add edit distance computation matrix to the log file
inline void edit_distance::log_edit_matrix(ofstream& ofs, string& str1, string& str2, MATRIX_TYPE matrix[MATRIX_SIZE][MATRIX_SIZE]) {
  ofs << str1 << endl;
  ofs << str2 << endl;
  ofs << endl;
  size_t nr = str1.length() + 1;
  size_t nc = str2.length() + 1;
  for(int c=0; c<nc; ++c) {
    if (c==0) ofs << "      ";
    ofs << c << " ";
  }
  ofs << endl;
  ofs << "      ";
  for(int c=0; c<nc; ++c) {
    if (c==0)
      ofs << "#";
    else
      ofs << str2[c-1];
    ofs << " ";
  }
  ofs << endl;
  for(int r=0; r<nr; ++r) {
    ofs << r << " ";
    if (r>0)
      ofs << str1[r-1];
    else
      ofs << "#";
    ofs << " | ";
    for(int c=0; c<nc; ++c) {
      ofs << matrix[r][c];
      ofs << " ";
    }
    ofs << " | " << endl;
  }
  ofs << "    ";
  for (int i=0; i<(2*nc+4); ++i)
  {
    ofs << "-";
  }
  ofs << endl;
  ofs << endl;
}
/// @brief optimized edit distance computation algorithm
/// @authors Joyanta and Al-Mamun
/// @param str1 first string
/// @param str2 second string
/// @param threshRem threshold remaining or bound on the edit distance to be returned
/// @return edit distance calculated.
/// @Description If the edit distance is greater than given threshold, return @c large_number
/// @note returned value is not always correct.
int edit_distance::edit_dist_J(string& str1, string& str2, int threshRem) {
  int dist	= threshRem;

  if(abs((int)(str1.length() - str2.length())) > dist)
    return large_number;
  else if(str1.compare(str2) == 0)
    return 0;
  else if(dist == 0)
    return large_number;
  else if((2 * dist + 1) >= max(str1.length(), str2.length()))
    return edit_dist_q(str1, str2, dist);
  else
  {
    string s1, s2;
    int row, col, diagonal;
    int i, j;

    if (str1.length() > str2.length())
    {
      s1 = str2;
      s2 = str1;
    }
    else
    {
      s1 = str1;
      s2 = str2;
    }

    row	 		= s1.length() + 1;
    col 		= 2 * dist + 1;
    diagonal 	= dist + s2.length() - s1.length();

    for(i = 0; i < dist + 1; i++)
    {
      for(j = dist - i; j < col; j++)
      {
        int e = ((int)s1[i - 1] == (int)s2[j - (dist - i) - 1]) ? 0 : 1;
        if (i == 0) {
          matArr[i][j]	= j - dist;
        }
        else if(j == (dist - i)) {
          matArr[i][j] 	= matArr[i - 1][j + 1] + 1;
        }
        else if(j != (col - 1))
        {

            matArr[i][j]	= min(min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                      (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1)),
                                      (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }
        else
        {
            matArr[i][j]	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }

        if((j == diagonal) && matArr[i][j] > dist)
          return large_number;
      }
    }

    for(i = dist + 1; i < s2.length() - dist + 1; i++)
    {
      for(j = 0; j < col; j++)
      {
        int e = ((int)s1[i - 1] == (int)s2[j + (i - dist) - 1]) ? 0 : 1;
        if(j == 0)
        {
            matArr[i][j]	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1));
        }
        else if(j != (col - 1))
        {
            matArr[i][j] 	= min(min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                      (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1)),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }
        else
        {
            matArr[i][j] 	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        }
        if((j == diagonal) && (matArr[i][j] > dist))
          return large_number;
      }
    }

    for(i = s2.length() - dist + 1; i < row; i++)
    {
      for(j = 0; j < col - i + s2.length() - dist; j++)
      {
        int e = ((int)s1[i - 1] == (int)s2[j + (i - dist) - 1]) ? 0 : 1;
        if(j == 0)
            matArr[i][j]	= min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                  (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1));
        else
            matArr[i][j] 	= min(min((MATRIX_TYPE)(matArr[i - 1][j] + e),
                                      (MATRIX_TYPE)(matArr[i - 1][j + 1] + 1)),
                                  (MATRIX_TYPE)(matArr[i][j - 1] + 1));
        if((j == diagonal) && (matArr[i][j] > dist))
          return large_number;
      }
    }
    return matArr[row - 1][diagonal];
  }
}
/// @brief Use pre-initialized edit distance matrix
/// The first row and first column are initialized only once by the constructor
/// @note see edit_dist_q for further details
int edit_distance::edit_dist_q2(string& str1, string& str2, int threshRem)
{
  int row, col, i, j;

  row	= (int) str1.length() + 1;
  col 	= (int) str2.length() + 1;

  for(i = 1; i < row; i++)
  {
    for(j = 1; j < col; j++)
    {
        if((int)str1[i-1] == (int)str2[j-1]) {
          MATRIX_TYPE temp = min(ED[i - 1][j - 1], (MATRIX_TYPE) (ED[i - 1][j] + 1));
          ED[i][j] = min(temp, ((MATRIX_TYPE) (ED[i][j - 1] + 1)));
        }
        else{
          MATRIX_TYPE temp = min((MATRIX_TYPE) (ED[i - 1][j - 1] + 1), (MATRIX_TYPE) (ED[i - 1][j] + 1));
          ED[i][j] = min(temp, ((MATRIX_TYPE) (ED[i][j - 1] + 1)));
        }

      if((row - col) == (i - j) && (ED[i][j] > threshRem))
      {
        return large_number;
      }
    }
  }
  return (ED[row - 1][col - 1]);
}
/// @brief Use pre-initialized edit distance matrix
/// The first row and first column are initialized only once by the constructor
/// @note See edit_dist_J for further details
int edit_distance::edit_dist_J2_log(string& str1, string& str2, int threshRem) {
  int &k = threshRem;
  int i=0, j=0;
  static ofstream ed_log = init_log_file();
  int edit_dist; /// edit distance return value
  if (abs((int) (str1.length() - str2.length())) > k)
  {
    edit_dist = large_number;
    goto edit_dist_J2_ret;
  }
  if (str1 == str2) {
    edit_dist = 0;
    goto edit_dist_J2_ret;
  }
  if (k == 0) {
    edit_dist = large_number;
    goto edit_dist_J2_ret;
  }
  if ((2 * k + 1) >= max(str1.length(), str2.length()))
  {
    edit_dist = edit_dist_q2(str1, str2, k);
    goto edit_dist_J2_ret;
  }
  {
    string s1, s2;
    int row, col;
    if (str1.length() > str2.length()) {
      s1 = str2;
      s2 = str1;
    } else {
      s1 = str1;
      s2 = str2;
    }
    int m = (int) s2.length();
    int n = (int) s1.length();
    int q = m - n;
    row = n + 1;
    for (i = 1; i < row; i++)
    {
      int j1 = max(i - k, 1);
      int j2 = min(k + i, m);
      for (j = j1; j <= j2; j++)
      {
        int e = ((int) s1[i - 1] == (int) s2[j - 1]) ? 0 : 1;
        int diag = ED[i - 1][j - 1] + e;
        int top = (j == (k + i)) ? large_number : ED[i - 1][j];
        int left = (i == (k+j)) ? large_number : ED[i][j - 1];
        int left_top = min(left, top) + 1;
        ED[i][j] = min(left_top, diag);
        if ((j == (q + i)) && (ED[i][j] > threshRem)) {
          return large_number;
        }
      }
    }
    edit_dist = ED[n][m];
    i = n;
    j = m;
  }
edit_dist_J2_ret:
  static int cnt = 0;
  if (cnt<10)
  {
    ed_log << "Threshold = " << threshRem << endl;
    log_edit_matrix(ed_log, str1, str2, ED);
    ed_log << "(i,j) = (" << i << "," << j << ")" << endl;
    ed_log << "edit distance = " << edit_dist << endl;
    ed_log << endl;
    cnt++;
  }
  else
  {
    ed_log.close();
  }
  return (edit_dist);
}
/// @brief Use pre-initialized edit distance matrix
/// The first row and first column are initialized only once by the constructor
/// @note See edit_dist_J for further details
int edit_distance::edit_dist_J2(string& str1, string& str2, int threshRem) {
  int &k = threshRem;
  if (abs((int) (str1.length() - str2.length())) > k)
    return large_number;
  if (str1 == str2)
    return 0;
  if (k == 0)
    return large_number;
  if ((k + k + 1) >= max(str1.length(), str2.length()))
    return edit_dist_q2(str1, str2, k);
  string s1, s2;
  int row, col;
  if (str1.length() > str2.length()) {
    s1 = str2;
    s2 = str1;
  } else {
    s1 = str1;
    s2 = str2;
  }
  int m = (int) s2.length();
  int n = (int) s1.length();
  int q = m - n;
  row = n + 1;
  for (int i = 1; i < row; i++)
  {
    int j1 = max(i - k, 1);
    int j2 = min(k + i, m);
    for (int j = j1; j <= j2; j++)
    {
      int e = ((int) s1[i - 1] == (int) s2[j - 1]) ? 0 : 1;
      int diag = ED[i - 1][j - 1] + e;
      int top = (j == (k + i)) ? large_number : ED[i - 1][j];
      int left = (i == (k+j)) ? large_number : ED[i][j - 1];
      int left_top = min(left, top) + 1;
      ED[i][j] = min(left_top, diag);
      if ((j == (q + i)) && (ED[i][j] > threshRem)) {
        return large_number;
      }
    }
  }
  return ED[n][m];
}

/// @brief Use pre-initialized edit distance matrix
/// The first row and first column are initialized only once by the constructor
/// @note See edit_dist_A for further details
int edit_distance::edit_dist_A2(string& str1, string& str2, int threshRem) {
  if (str1==str2) return 0;
  int &k = threshRem;
  if (k == 0) return large_number;
  int n = (int) str1.length();
  int m = (int) str2.length();
  bool swapped = false;
  if (n>m) {  // ensure m >= n
    swap(m,n);
    swapped = true;
  }
  int q = m-n;
  if (q > k) return large_number;
  int p = 0;
  while ((p<n) && (str1[p]==str2[p])) p++;
  if(p==n) return q;
  n -= p;
  m -= p;
  int kp = (k + q)/2; // highest diagonal
  int kpb = (k - q)/2; // lowest diagonal
  if((kpb + kp + 1) >= m) {
    string s1 = str1.substr(p);
    string s2 = str2.substr(p);
    return edit_dist_q2(s1, s2, k);
  }
  string& s1 = str1;
  string& s2 = str2;
  if (swapped) swap(s1,s2);
  int row = n + 1;
  for (int i = 1; i < row; i++)
  {
    int j1 = max(i - kpb, 1);
    int j2 = min(kp + i, m);
    for (int j = j1; j <= j2; j++)
    {
      int e = (s1[p + i - 1] == s2[p + j - 1]) ? 0 : 1;
      int diag = ED[i - 1][j - 1] + e;
      int top = (j == (kp + i)) ? large_number : ED[i - 1][j];
      int left = (i == (kpb + j)) ? large_number : ED[i][j - 1];
      int left_top = min(left, top) + 1;
      ED[i][j] = (MATRIX_TYPE) min(left_top, diag);
      if ((j == (q + i)) && (ED[i][j] > threshRem)) {
        return large_number;
      }
    }
  }
  return ED[n][m];
}

/// @note See edit_dist_A2 for further details
int edit_distance::edit_dist_A3(string& str1, string& str2, int threshRem) {
  if (str1==str2) return 0;
  int &k = threshRem;
  if (k == 0) return large_number;
  int n = (int) str1.length();
  int m = (int) str2.length();
  bool swapped = false;
  if (n>m) {  // ensure m >= n
    swap(m,n);
    swapped = true;
  }
  int q = m-n;
  if (q > k) return large_number;
  int p = 0;
  while ((p<n) && (str1[p]==str2[p])) p++;
  if(p==n) return q;
  n -= p;
  m -= p;
  int h = (k + q) / 2; // highest diagonal
  int l = (k - q) / 2; // lowest diagonal
  int w = l + h + 1; // width of the band
  if(w >= m) {
    string s1 = str1.substr(p);
    string s2 = str2.substr(p);
    return edit_dist_q2(s1, s2, k);
  }
  string& s1 = str1;
  string& s2 = str2;
  if (swapped) swap(s1,s2);
  MATRIX_TYPE* r1 = (MATRIX_TYPE*) malloc(w*sizeof(MATRIX_TYPE));
  MATRIX_TYPE* r2 = (MATRIX_TYPE*) malloc(w*sizeof(MATRIX_TYPE));
  MATRIX_TYPE x = 0;
  MATRIX_TYPE *cr=r1, *nr=r2;
  bool toggle=true;
  for (int i = l; i < w; i++) { // initialize edit matrix row 0
      cr[i] = x++;
      nr[i] = 0;
  }
  for (int i = 1; i <= n; i++)
  {
    int j1 = ((i-l) > 1) ? (i-l) : 1;
    int j2 = ((h+i) < m) ? (h+i) : m;
    if (i<=l) {nr[l-i] = (MATRIX_TYPE)i;}  // edit matrix column 0
    for (int j = j1; j <= j2; j++)
    {
      int jli = j + l - i;
      int e = (s1[p + i - 1] == s2[p + j - 1]) ? 0 : 1;
      int diag = cr[jli] + e;
      int top = (j == (h + i)) ? large_number : cr[jli+1];
      int left = (i == (l + j)) ? large_number : nr[jli-1];
      int left_top = ((left < top) ? left : top) + 1;
      nr[jli] = (MATRIX_TYPE) ((left_top < diag) ? left_top : diag);
      if ((j == (q + i)) && (nr[jli] > threshRem)) {
        return large_number;
      }
    }
    if (toggle) {
        cr=r2; nr=r1; toggle=false;
    } else {
        cr=r1; nr=r2; toggle=true;
    }
  }
  return cr[m+l-n];
}

/// @author Ahmed Soliman
/// @brief recoding of Ukkonen's algorithm originally implemented by Sune Simonsen using Java Script
/// @details see https://github.com/sunesimonsen/ukkonen for details
int edit_distance::edit_dist_Ukkonen(string& a, string& b, int threshold) {
  // if (threshold<0) return large_number;
  threshold++;
  if (a == b) {
    return 0;
  }
  if (a.length() > b.length()) {
    // Swap a and b so b is longer or same length as a
    string tmp = a;
    a = b;
    b = tmp;
  }
  int aLen = (int) a.length();
  int bLen = (int) b.length();
  // Performing suffix trimming:
  // We can linearly drop suffix common to both strings since they
  // don't increase distance at all
  while (aLen > 0 && a[aLen - 1] == b[bLen - 1]) {
    aLen--;
    bLen--;
  }
  if (aLen == 0) {
    return (bLen < threshold) ? bLen : large_number;
  }
  // Performing prefix trimming
  // We can linearly drop prefix common to both strings since they
  // don't increase distance at all
  int tStart = 0;
  while (tStart < aLen && a[tStart] == b[tStart]) {
    tStart++;
  }
  aLen -= tStart;
  bLen -= tStart;
  if (aLen == 0) {
    return (bLen < threshold) ? bLen : large_number;
  }
//  threshold = (bLen < threshold) ? bLen : threshold;
  int dLen = bLen - aLen;
  if (threshold < dLen) {
    return large_number;
  }
  // floor(min(threshold, aLen) / 2)) + 2
  int ZERO_K = (((aLen < threshold) ? aLen : threshold) >> 1) + 2;
  int arrayLength = dLen + ZERO_K * 2 + 2;
  vector<int> currentRow(arrayLength, -1);
  vector<int> nextRow(arrayLength, -1);
  std::vector<unsigned char> aCharCodes(aLen);
  std::vector<unsigned char> bCharCodes(bLen);
  int i = 0;
  int tt = tStart;
  while (i < aLen) {
    aCharCodes[i] = a[tt];
    bCharCodes[i] = b[tt];
    i++;
    tt++;
  }
  while (i < bLen) {
    bCharCodes[i++] = b[tt++];
  }
  int j = 0;
  int conditionRow = dLen + ZERO_K;
  int endMax = conditionRow << 1;
  do {
    j++;
    std::vector<int> tmp = currentRow;
    currentRow = nextRow;
    nextRow = tmp;
    int start;
    int previousCell;
    int currentCell = -1;
    int nextCell;
    if (j <= ZERO_K) {
      start = -j + 1;
      nextCell = j - 2;
    } else {
      start = j - (ZERO_K << 1) + 1;
      nextCell = currentRow[ZERO_K + start];
    }
    int end;
    if (j <= conditionRow) {
      end = j;
      nextRow[ZERO_K + j] = -1;
    } else {
      end = endMax - j;
    }
    for (int k = start, rowIndex = start + ZERO_K; k < end; k++, rowIndex++) {
      previousCell = currentCell;
      currentCell = nextCell;
      nextCell = currentRow[rowIndex + 1];
      // max(t, previousCell, nextCell + 1)
      int t = currentCell + 1;
      t = std::max(t, previousCell);
      t = std::max(t, nextCell + 1);
      while (t < aLen && t + k < bLen && aCharCodes[t] == bCharCodes[t + k]) {
        t++;
      }
      nextRow[rowIndex] = t;
    }
  } while (nextRow[conditionRow] < aLen && j <= threshold);

  j = ((j-1)<threshold) ? j-1 : large_number;

  return j;
}
