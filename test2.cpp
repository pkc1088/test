#include <iostream>
#include <cstring>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

int p, n, m, t, maxsum;
vector <int> vec;
vector <int> dpvec;

int main() {

	int a = 1234;	
	ios_base::sync_with_stdio(false);
	cin.tie(NULL);
	cout.tie(NULL);
	int st, ed, noi;

	cin >> p >> n >> m;
	vec.assign(n, p);
	dpvec.assign(n, 0);

	for (int i = 0; i < m; i++) {
		cin >> t;
		for (int i = 0; i < t; i++) {
			cin >> st >> ed >> noi;
			for (int i = st; i < ed; i++) {
				vec[i] += noi;
			}
		}
	}

	dpvec[0] = vec[0];
	maxsum = vec[0];

	for (int i = 1; i < vec.size(); i++) {
		dpvec[i] = max(dpvec[i - 1] + vec[i], vec[i]);
		maxsum = max(maxsum, dpvec[i]);
	}

    cout << maxsum << endl;
	
	return 0;
}















