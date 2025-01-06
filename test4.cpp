#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <climits>
using namespace std;

struct Order {
    int stx, sty, edx, edy;
    bool picked, delivered;
};

int manhattan(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

void TSP(vector<Order>& od, int curX, int curY, vector<int>& curR, vector<int>& rstR, int& minD, int curD, int food) {
    if (curR.size() == od.size() * 2) {
        if (curD < minD || (curD == minD && lexicographical_compare(curR.begin(), curR.end(), rstR.begin(), rstR.end()))) { 
            minD = curD;
            rstR = curR;
        }
        return;
    }

    for (int i = 0; i < od.size(); ++i) {
        if (!od[i].picked && food < 2) {
            od[i].picked = true;
            curR.push_back(i + 1);
            TSP(od, od[i].stx, od[i].sty, curR, rstR, minD, 
                curD + manhattan(curX, curY, od[i].stx, od[i].sty), food + 1);
            curR.pop_back();
            od[i].picked = false;
        }
        if (od[i].picked && !od[i].delivered) {
            od[i].delivered = true;
            curR.push_back(-(i + 1));
            TSP(od, od[i].edx, od[i].edy, curR, rstR, minD, 
                curD + manhattan(curX, curY, od[i].edx, od[i].edy), food - 1);
            curR.pop_back();
            od[i].delivered = false;
        }
    }
}

int main() {
    int N;
    cin >> N;

    vector<Order> od(N);
    vector<int> curR, rstR;
    int minD = INT_MAX;
    
    for (int i = 0; i < N; i++) {
        cin >> od[i].stx >> od[i].sty >> od[i].edx >> od[i].edy;
        od[i].picked = false;
        od[i].delivered = false;
    }

    TSP(od, 500, 500, curR, rstR, minD, 0, 0);

    for (int step : rstR) cout << step << " ";
    
    cout << endl << minD;

    return 0;
}


