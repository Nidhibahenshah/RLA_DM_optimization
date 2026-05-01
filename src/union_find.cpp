
#include "union_find.h"

using namespace std;

namespace uf {

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
/// Find root of a point in components
int find_root(int pointID, std::vector<int> &parentArr) {
    if (parentArr[pointID] != pointID)
        parentArr[pointID] = uf::find_root(parentArr[pointID], parentArr);
    return parentArr[pointID];
}
#pragma clang diagnostic pop

/// Unify two components rooted at rootU and rootV
void make_equivalent(int rootU, int rootV, vector<int> &parentArr, vector<int> &weightArr) {
    if (weightArr[rootU] < weightArr[rootV])
        parentArr[rootU] = rootV;
    else if (weightArr[rootU] > weightArr[rootV])
        parentArr[rootV] = rootU;
    else {
        parentArr[rootV] = rootU;
        weightArr[rootU] = weightArr[rootU] + 1;
    }
}

///  Find connected components in the linking graph. Vertices in the linking graph represent records while edges
/// represent connections/links among the corresponding records.
void find_connected_components(std::vector<std::vector<int>> &clusterExactIndArr,
                                   std::vector<std::vector<int>> &edgeArr,
                                   std::vector<int> &parentArr) {
    int i, rootU, rootV;
    vector<int> weightArr;
    auto pointTotal = clusterExactIndArr.size();
    for (i = 0; i < pointTotal; ++i) {
        parentArr.push_back(i);
        weightArr.push_back(0);
    }
    auto edgeTotal = edgeArr.size();
    for (i = 0; i < edgeTotal; ++i) {
        rootU = find_root(edgeArr[i][0], parentArr);
        rootV = find_root(edgeArr[i][1], parentArr);
        if (rootU != rootV) {
            uf::make_equivalent(rootU, rootV, parentArr, weightArr);
        }
    }
}

} //namespace uf
