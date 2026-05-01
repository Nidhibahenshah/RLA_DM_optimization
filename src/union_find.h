
#ifndef RLA_CL_UNION_FIND_H
#define RLA_CL_UNION_FIND_H

#include <vector>

namespace uf {

    int find_root(int pointID, std::vector<int> &parentArr);

    void make_equivalent(int rootU, int rootV,
                         std::vector<int> &parentArr,
                         std::vector<int> &weightArr);

    void find_connected_components(std::vector<std::vector<int> >& clusterExactIndArr,
                                   std::vector<std::vector<int> >& edgeArr,
                                   std::vector<int>& parentArr);

}

#endif //RLA_CL_UNION_FIND_H
