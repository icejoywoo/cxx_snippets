#include <gtest/gtest.h>
#include <vector>

/**
 * leetcode:
 * 1. https://leetcode.cn/problems/number-of-islands/solutions/211211/dao-yu-lei-wen-ti-de-tong-yong-jie-fa-dfs-bian-li-/?envType=study-plan-v2&envId=top-interview-150
 * 2. https://leetcode.cn/problems/surrounded-regions/
 */

namespace d2 {
class UnionFind {
public:
    UnionFind(std::vector<std::vector<char>>& grid) {
        m = grid.size();
        n = grid[0].size();
        count = 0;
        {
            std::vector<int> tmp(m * n, -1);
            parent.swap(tmp);
        }
        {
            std::vector<int> tmp(m * n, 0);
            rank.swap(tmp);
        }

        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                if (grid[i][j] == '1') {
                    parent[row2index(i, j)] = row2index(i, j);
                    count++;
                }
            }
        }
    }

    int row2index(int i, int j) {
        return i * n + j;
    }

    int findRoot(int i) {
        int p = parent[i];
        while (p != parent[p]) {
            p = parent[p];
        }
        return p;
    }

    void unionOne(int x, int y) {
        int rootx = findRoot(x);
        int rooty = findRoot(y);

        if (rootx != rooty) {
            if (rank[rootx] > rank[rooty]) {
                parent[rooty] = rootx;
            } else if (rank[rootx] < rank[rooty]) {
                parent[rootx] = rooty;
            } else { // rank[rootx] == rank[rooty]
                parent[rooty] = rootx;
                count--;
            }
        }
    }

    int getCount() {
        return count;
    }
private:
    std::vector<int> parent;
    std::vector<int> rank;
    int m; // rows
    int n; // columns
    int count;
};
} // namespace d2

namespace d1 {
// https://leetcode.cn/problems/satisfiability-of-equality-equations/
class UnionFind {
public:
    UnionFind() {
        roots.resize(26);
        for (int i = 0; i < 26; i++) {
            roots[i] = i;
        }
    }

    int find(int index) {
        if (index == roots[index]) {
            return index;
        }
        roots[index] = find(roots[index]);
        return roots[index];
    }

    void unite(int index1, int index2) {
        roots[find(index1)] = find(index2);
    }
private:
    std::vector<int> roots;
};
} // namespace d1

TEST(UnionFindTest, Basic) {
    std::vector<std::vector<char>> grid{
        {'1', '1', '1', '1', '0'},
        {'1', '1', '0', '1', '0'},
        {'1', '1', '0', '0', '0'},
        {'0', '0', '0', '0', '0'}
    };

    d2::UnionFind uf(grid);
    EXPECT_EQ(9, uf.getCount());

    std::vector<std::pair<int, int>> directions{
        {-1, 0},
        {0, -1},
        {0, 1},
        {1, 0}
    };

    int n = grid.size();
    int m = grid[0].size();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (grid[i][j] == '0') continue;
            for (auto d : directions) {
                if (i+d.first < 0 || i+d.first >= n) continue;
                if (j+d.second < 0 || j+d.second >= m) continue;
                if (grid[i+d.first][j+d.second] == '1') {
                    uf.unionOne(uf.row2index(i, j), uf.row2index(i+d.first, j+d.second));
                }
            }
        }
    }

    EXPECT_EQ(1, uf.getCount());
}