#include <iostream>
#include <cstring>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;
int method, k, n, el, comp_count = 0, depth = 0;
vector<int> vec;
/*
// 분할 단계: pivot을 기준으로 작은 원소들은 왼쪽으로, 큰 원소들은 오른쪽으로 배치
int partition(int low, int high) {
    int pivot = vec[low]; // pivot을 배열의 첫 번째 원소로 선택
    int i = high + 1; // 큰 원소들을 모으는 위치

    for (int j = high; j > low; j--) {
        if (vec[j] >= pivot) {
        i--;
            swap(vec[i], vec[j]); // 큰 원소를 오른쪽으로 이동
        }
    }

    swap(vec[i - 1], vec[low]); // pivot을 올바른 위치로 이동
    return i - 1; // pivot의 위치 반환
}

// 정복 단계: 분할된 부분 배열을 재귀적으로 정렬
void quickSort(int low, int high) {
    if (low < high) {
        int pivotIndex = partition(low, high); // 분할
        quickSort(low, pivotIndex - 1); // 왼쪽 부분 배열을 재귀적으로 정렬
        quickSort(pivotIndex + 1, high); // 오른쪽 부분 배열을 재귀적으로 정렬
    }
}
*/
void print() {
    cout << "\nSorted array soo far\n";
    for (int num : vec) {
        cout << num << " ";
    }
    cout << endl;
    exit(0);
}
/*
int partition(int low, int high) {
    int pivot = low; // 피봇을 배열의 맨 앞 원소로 설정
    int left = low + 1; // 왼쪽 포인터 초기 위치 설정
    int right = high; // 오른쪽 포인터 초기 위치 설정

    while (left <= right) {
        // pivot보다 큰 값을 찾을 때까지 왼쪽으로 이동
        while (left <= right && vec[left] <= vec[pivot]) left++;

        // pivot보다 작은 값을 찾을 때까지 오른쪽으로 이동
        while (vec[right] >= pivot) right--;

        if (left < right) 
            swap(left, right);
    }
    
    vec[low] = vec[right];
    vec[right] = pivot;
    //depth++;
    //if(right == k) print();
    // pivot의 위치 반환
    return right;
}

*/
void quickSort(int start, int end) {
    if(start >= end){
        return; 
    }
    
    int pivot = start;
    int i = pivot + 1; // 왼쪽 출발 지점 
    int j = end; // 오른쪽 출발 지점
    int temp;
        
    while(i <= j){
        
        // 포인터가 엇갈릴때까지 반복
        while(i <= end && vec[i] <= vec[pivot])    i++;
        while(j > start && vec[j] >= vec[pivot])    j--;
        
        if(i > j){
            // 엇갈림 (j번째와 pivot 스왑)
            temp = vec[j];
            vec[j] = vec[pivot];
            vec[pivot] = temp;
        } else {
            // i번째와 j번째를 스왑
            temp = vec[i];
            vec[i] = vec[j];
            vec[j] = temp;
        }
    } 
    
    if(depth == k) print();
    depth++;
    // 분할 계산
    //if(end + 1 == k) print();
    quickSort(start, j - 1);
    //depth++;
    quickSort(j + 1, end);
    //depth++;
}

int main() {
    cin >> method >> k >> n;
    for (int i = 0; i < n; i++) {
        cin >> el;
        vec.push_back(el);
    }
    cout<< "heyy\n";
    k--;
    quickSort(0, n - 1); // 퀵 정렬 수행
/*
    cout << "\nSorted array: ";
    for (int num : vec) {
        cout << num << " ";
    }
    cout << endl;
*/
    return 0;
}