#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>
using namespace std;

#define NumberRecords   20000  
#define NumberQueries   100
#define BPLUS_TREE_ORDER 1000

struct Student {
    char name[20];
    unsigned ID;
    float score;
    char dept[10];
};

struct BPTnode {
    bool isLf;
    vector<long> offsets;
    vector<float> keys;
    vector<BPTnode*> chdr;
    BPTnode(bool lf = true) : isLf(lf) {}
};

class BPT {
public:
    BPT();
    void insert(float key, long offset);
    void search(float minKey, float maxKey, vector<long>& result);
private:
    BPTnode* rt;
    void intoinsert(float key, long offset, BPTnode* nd, BPTnode** newNd, float* newKey);
    void intoBranch(BPTnode* nd, BPTnode** newNd, float* newKey);
    void branchLeaf(BPTnode* lf, BPTnode** newLeaf, float* newKey);
};

BPT::BPT() { rt = new BPTnode(true); }

void BPT::insert(float key, long offset) {
    BPTnode* newNd = nullptr;
    float newKey;
    intoinsert(key, offset, rt, &newNd, &newKey);
    if (newNd) {
        BPTnode* newRt = new BPTnode(false);
        newRt->keys.push_back(newKey);
        newRt->chdr.push_back(rt);
        newRt->chdr.push_back(newNd);
        rt = newRt;
    }
}

void BPT::intoinsert(float key, long offset, BPTnode* nd, BPTnode** newNd, float* newKey) {
    if (nd->isLf) {
        nd->keys.insert(lower_bound(nd->keys.begin(), nd->keys.end(), key), key);
        nd->offsets.insert(
            nd->offsets.begin() 
                + (nd->keys.end() 
                    - lower_bound(nd->keys.begin(), nd->keys.end(), key) - 1
                  )
        , offset);
        
        if (nd->keys.size() > BPLUS_TREE_ORDER - 1) branchLeaf(nd, newNd, newKey);
    } 
    else {
        int i;
        for (i = 0; i < nd->keys.size(); i++) if (key < nd->keys[i]) break; // <
        
        intoinsert(key, offset, nd->chdr[i], newNd, newKey);
        if (*newNd) {
            nd->keys.insert(nd->keys.begin() + i, *newKey);
            nd->chdr.insert(nd->chdr.begin() + i + 1, *newNd);
            if (nd->keys.size() > BPLUS_TREE_ORDER - 1) intoBranch(nd, newNd, newKey);
            else *newNd = nullptr; 
        }
    }
}

void BPT::branchLeaf(BPTnode* lf, BPTnode** newLeaf, float* newKey) {
    *newLeaf = new BPTnode(true);
    int mid = lf->keys.size() / 2;
    *newKey = lf->keys[mid];
    (*newLeaf)->keys.assign(lf->keys.begin() + mid, lf->keys.end());
    (*newLeaf)->offsets.assign(lf->offsets.begin() + mid, lf->offsets.end());
    lf->keys.resize(mid);
    lf->offsets.resize(mid);
}

void BPT::intoBranch(BPTnode* nd, BPTnode** newNd, float* newKey) {
    *newNd = new BPTnode(false);
    int mid = nd->keys.size() / 2;
    *newKey = nd->keys[mid];
    (*newNd)->keys.assign(nd->keys.begin() + mid + 1, nd->keys.end());
    (*newNd)->chdr.assign(nd->chdr.begin() + mid + 1, nd->chdr.end());
    nd->keys.resize(mid);
    nd->chdr.resize(mid + 1);
}

void BPT::search(float minKey, float maxKey, vector<long>& result) {
    BPTnode* cur = rt;
    while (!cur->isLf) {
        int i;
        for (i = 0; i < cur->keys.size(); i++) if (minKey < cur->keys[i]) break;   
        cur = cur->chdr[i];
    }
    while (cur) {
        for (int i = 0; i < cur->keys.size(); i++) 
            if (cur->keys[i] >= minKey && cur->keys[i] <= maxKey) result.push_back(cur->offsets[i]);
        cur = cur->chdr.empty() ? nullptr : cur->chdr.back();
    }
}
unordered_map<unsigned, long> idIndex;
BPT scoreBPT;

bool insert(char* name, unsigned* ID, float* score, char* dept) { 
    fstream file("student.dat", ios::in | ios::out | ios::binary | ios::app);
    fstream idxfile("score.idx", ios::in | ios::out | ios::binary | ios::app);
    
    if (!file || !idxfile) { cerr << "Cannot open student.dat or score.idx file." << endl; return false; }
    if (idIndex.find(*ID) != idIndex.end()) return false; 

    Student student;
    strncpy(student.name, name, 20);
    student.ID = *ID;
    student.score = *score;
    strncpy(student.dept, dept, 10);

    file.seekp(0, ios::end);
    long offset = file.tellp();
    file.write(reinterpret_cast<char*>(&student), sizeof(Student));    // 38
    file.close();

    idIndex[*ID] = offset;
    scoreBPT.insert(*score, offset);
    idxfile.seekp(0, ios::end);
    idxfile.write(reinterpret_cast<char*>(&offset), sizeof(long)); 
    idxfile.close();

    return true;
}

bool searchByID(char* name, unsigned ID, float* score, char* dept) {
    fstream file("student.dat", ios::in | ios::binary);

    if (!file) {cerr << "student.dat file open error" << endl; return false;}

    auto it = idIndex.find(ID);
    if (it == idIndex.end()) {return false;}

    long offset = it->second;

    file.seekg(offset, ios::beg);
    Student student;
    file.read(reinterpret_cast<char*>(&student), sizeof(Student)); //38
    file.close();

    strncpy(name, student.name, 20);
    *score = student.score;
    strncpy(dept, student.dept, 10);
    return true;
}

bool searchByScore(unsigned* number, unsigned* IDs, float scoreMin, float scoreMax) {
    fstream file("student.dat", ios::in | ios::binary);
    if (!file) { cerr << "student.dat file open error" << endl; return false; }

    *number = 0;
    vector<long> offsets;
    scoreBPT.search(scoreMin, scoreMax, offsets);
    for (long offset : offsets) {
        file.seekg(offset, ios::beg);
        Student student;
        file.read(reinterpret_cast<char*>(&student), sizeof(Student)); // 38
        IDs[*number] = student.ID;   
        (*number)++; 
    }
    file.close();
    return *number > 0;
}

class LIKStudent {
public:
    string      name;
    unsigned    ID;
    float       score;
    string      dept;
};

int main()
{
    ifstream    ifs("file.txt", ios::in);
    unsigned    ID;
    string      name;
    float       score;
    string      dept;
    char        cname[21];
    char        cdept[11];
    LIKStudent  stud[NumberRecords];
    clock_t     ts, te;
    double      durationInsertion;


    system("del student.dat");

    // insert records
    for(int i=0;i<NumberRecords;i++) {
        ifs>>ID>>name>>score>>dept;
        stud[i].ID=ID; stud[i].name=name; stud[i].dept=dept; stud[i].score=score;
    }
    ts=clock();
    for(int i=0;i<NumberRecords;i++) {          
        strcpy(cname,stud[i].name.c_str()); 
        strcpy(cdept,stud[i].dept.c_str());
        if(insert(cname, &stud[i].ID, &stud[i].score, cdept)!=true) cout<<"In Insert "<<i<<": Error\n";
    }
    ifs.close();
    te=clock();
    durationInsertion=(double)(te-ts)/CLOCKS_PER_SEC;
    cout<<"Insertion time: "<<durationInsertion<<endl;


    // search by ID
    ts=clock();
    ifstream     ifs_ID_Query("IDQuery.txt", ios::in);
    for(int i=0;i<NumberQueries;i++) {
        ifs_ID_Query>>ID;
        if(!searchByID(cname, ID, &score, cdept)) cout<< i+1 << " search by ID: not found\n";
        else cout << ID << " " << cname << " " << score << " " << cdept << endl;
    }
    ifs_ID_Query.close();
    te=clock();
    float durationSearchByID=(double)(te-ts)/CLOCKS_PER_SEC;
    cout<<"search by ID time: "<<durationSearchByID<<endl;
    ts=clock();


    // search by score
    ifstream     ifs_Score_Query("ScoreQuery.txt", ios::in);
    float        scoreMin, scoreMax;
    unsigned     number;
    unsigned     IDs[NumberRecords];
    for(int i=0;i<NumberQueries;i++) {
        ifs_Score_Query>>scoreMin>>scoreMax;
        if(searchByScore(&number, IDs, scoreMin, scoreMax)) {
            cout << i << endl;
            for(unsigned m=0;m<number;m++) cout<< IDs[m] << endl;
        }
        else cout<< i+1 << " search by score : not found\n";
    }
    te=clock();
    float durationSearchByScore=(double)(te-ts)/CLOCKS_PER_SEC;
    cout<<"search by score time: "<<durationSearchByScore<<endl;
    float total_time=durationInsertion+durationSearchByID+durationSearchByScore;
    cout<<"total time: "<<total_time<<endl;
    
  
    
    return 0;
}
