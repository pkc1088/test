#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// 서브 트리의 vertex 개수 구하는 함수(부모,자식 배열이랑 vertex, 크기 인자로)
int num_under(string parent[],string child[],string person,int N){
    int num=0; //vertex 개수
    for(int i=0;i<N-1;i++){
        if(parent[i]==person){ //person이 부모인 경우
            //자식으로 재귀 함수 실행
            num=num+1+num_under(parent,child,child[i], N);
        }
    }
    return num; //vertex 수 반환
}

//정점으로부터 level 구하는 함수(부모, 자식 배열이랑 vertex, 크기 인자로)
int get_depth(string parent[],string child[],string person,int N){
    int depth=0; //level
    //자식-부모 확인하며 정점까지 올라가기
    for(int i=0;i<N-1;i++){
        if(child[i]==person){
            depth++;
            person=parent[i];
            i=-1;
        }
    }
    return depth;//level 반환
}

//아래로 계층 구하는 함수(부모, 자식 배열이랑 vertex, 크기, 계층 인자로)
int get_under_depth(string parent[],string child[],string person,int N,int depth){
    int max_depth=depth; //최대 계층 변수
    //찾는 vertex를 찾는 반복문
    for(int i=0;i<N-1;i++){
        if(parent[i]==person){
            //찾았을 때 계층을 1 증가시켜 재귀함수 실행
            int temp=get_under_depth(parent,child,child[i],N,depth+1);
            if(max_depth<temp){//더 큰 계층을 받아옴
                max_depth=temp;
            }
        }
    }
    return max_depth;//계층 반환
}

//정점을 구하는 함수(부모, 자식 배열이랑 크기를인자로)
string get_boss(string parent[],string child[],int N){
    //정점으로부터 level이 1인 vertex의 부모를 반환
    for(int i=0;i<N-1;i++){
        if(get_depth(parent,child,child[i],N)==1) 
            return parent[i];
    }
    return NULL;
}

//필요한 모든 배열을 swap하는 함수(인자로 swap할 배열들과 index를 인자로)
void swap_arr(string parent[],string child[],int childs[],int top[],int bottom[], int i){
    swap(parent[i],parent[i+1]);
    swap(child[i],child[i+1]);
    swap(childs[i],childs[i+1]);
    swap(top[i],top[i+1]);
    swap(bottom[i],bottom[i+1]);
}

int main(){
    int N;//입력 크기
    cin>>N;
    string parent[N-1];//부모 배열
    string child[N-1];//자식 배열
    int num_childs[N-1];//서브 트리의 vertex 수
    int top[N-1];//위로 촌수
    int bottom[N-1];//아래로 최대 촌수
    for(int i=0;i<N-1;i++){
        cin>>child[i]>>parent[i];//배열에 값 대입
    }
    for(int i=0;i<N-1;i++){//배열에 값 대입
        num_childs[i]=num_under(parent,child,child[i],N);
        top[i]=get_depth(parent,child,child[i],N);
        bottom[i]=get_under_depth(parent,child,child[i],N,0);
    }
    bool sorted=true;
    while(sorted==true){
        sorted=false;
        for(int i=0;i<N-2;i++){//정렬 과정
            //서브 트리의 vertex가 앞이 더 작으면 스왑
            if(num_childs[i]<num_childs[i+1]){
                swap_arr(parent,child,num_childs,top,bottom,i);
                sorted=true;
            }
            else if(num_childs[i]==num_childs[i+1]){
                //위로 촌수가 앞이 클 경우 스왑
                if(top[i]>top[i+1]){
                    swap_arr(parent,child,num_childs,top,bottom,i);
                    sorted=true;
                }
                else if(top[i]==top[i+1]){
                    //아래로 촌수가 앞이 작을 경우 스왑
                    if(bottom[i]<bottom[i+1]){
                        swap_arr(parent,child,num_childs,top,bottom,i);
                        sorted=true;
                    }
                    else if(bottom[i]==bottom[i+1]){
                        if(child[i].compare(child[i+1])>0){
                            swap_arr(parent,child,num_childs,top,bottom,i);
                            sorted=true;
                        }
                    }
                }
            }
        }
    }


    //출력
    cout<<get_boss(parent,child,N)<<endl;
    for(int i=0;i<N-1;i++){
        cout<<child[i]<<endl;
    }
}