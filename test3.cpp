#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <algorithm>
using namespace std;

class Student {
public:
    char name[20];
    unsigned ID;
    float score;
    char dept[10];

    Student() { } 

    Student(char* name, unsigned ID, float score, char* dept) {
        strcpy(this->name, name);
        this->ID = ID;
        this->score = score;
        strcpy(this->dept, dept);
    }
};

bool search(char* name, unsigned ID, float *score, char* dept) {
	ifstream file("student.dat", ios::binary);
    if(!file) return false; 
    
    Student temp;
    while(file.read(reinterpret_cast<char*>(&temp), sizeof(Student))) {
        if(temp.ID == ID) { // temp 객체로 읽은 ID와 search의 매개변수 ID가 일치
            file.close();
            return true;
        }
    }
    
    file.close();
    return false;
}

bool insert(char* name, unsigned int *ID, float *score, char* dept) {
    fstream file("student.dat", ios::in | ios::out | ios::binary);
    if(!file) { // 해당 dat 파일이 존재하지 않으면 파일 생성 후 레코드 입력
        ofstream newFile("student.dat", ios::binary);
        Student student(name, *ID, *score, dept);
        newFile.write(reinterpret_cast<const char*>(&student), sizeof(Student));
        return true;
    }

//
    float temp_score = *score;
    unsigned temp_ID = *ID;
    char temp_name[20]; strcpy(temp_name, name);
    char temp_dept[10]; strcpy(temp_dept, dept);
    if(search(temp_name, temp_ID, &temp_score, temp_dept)) {     // 중복된 레코드가 있는지 확인 
        file.close();   
        return false; 
    }
//    
    Student student(name, *ID, *score, dept);  // 저장할 실제 데이터
    Student temp;                           // 파일에서 읽은 데이터를 임시 저장할 객체
    file.seekg(0, ios::beg); 
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(Student))) { 
        if (temp.ID == 0) { // ID가 0인 레코드는 drop된 레코드 임으로 그 위치에 레코드 입력
            file.seekp(-static_cast<long>(sizeof(Student)), ios::cur); // Move back to the empty slot
            file.write(reinterpret_cast<char*>(&student), sizeof(Student)); // Write the new record
            file.close();
            return true;
        }
    }
    
    // 중복된 레코드가 아니고 빈 공간이 없는 경우 파일 끝에 입력
    file.clear();               // EOF flag 를 초기화 시킴
    file.seekp(0, ios::end);    //커서를 파일 끝으로 이동
    file.write(reinterpret_cast<const char*>(&student), sizeof(Student));
    file.close();
    return true;
} 

bool read(int n, char* name, unsigned int *ID, float *score, char* dept) {
    ifstream file("student.dat", ios::binary);
    if(!file) return false;

    Student temp;
    file.seekg(n * sizeof(Student), ios::beg); // 40 바이트 * n 번째
    if (file.fail()) {  // 위치 이동 실패했거나 파일 끝을 넘어선 경우
        file.close(); 
        return false; 
    }
    
    file.read(reinterpret_cast<char*>(&temp), sizeof(Student));

    //cout << "Name : " << temp.name << ", ID : " << temp.ID << ", score : " << temp.score << ", dept : " << temp.dept << endl;    

    if (file.gcount() != sizeof(Student)) { // 요청한 만큼 데이터 읽지 못했을 경우
        file.close(); 
        return false; 
    }

    file.close();
	return true;
}

bool drop(char* name, unsigned ID, float *score, char* dept) {
    fstream file("student.dat", ios::binary | ios::in | ios::out);
    if(!file) return false;    

    Student temp;
    while(file.read(reinterpret_cast<char*>(&temp), sizeof(Student))) {
        if(temp.ID == ID) {
            temp.ID = 0;                //ID를 0으로 설정해서 빈 공간을 표시함
            strcpy(temp.name, "");      //나머지 변수들도 초기화 시킴
            temp.score = 0.0;
            strcpy(temp.dept, "");

            // 현재 위치로부터 한 레코드 크기 전으로 이동
            file.seekp(-static_cast<long>(sizeof(Student)), ios::cur);
            // 변경된 레코드를 다시 파일에 씀
            file.write(reinterpret_cast<char*>(&temp), sizeof(Student)); 
            file.close();
            return true;
        }
    }

	return true;
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
    char        cname[20];
    char        cdept[10];
    LIKStudent  stud[10000];

    system("del student.dat");
    for(int i=0;i<10000;i++) {
        ifs>>ID>>name>>score>>dept;
        stud[i].ID=ID; stud[i].name=name; stud[i].dept=dept; stud[i].score=score;
    }


    for(int i=0;i<10000;i++) {
        strcpy(cname,stud[i].name.c_str());
        strcpy(cdept,stud[i].dept.c_str());
        if(insert(cname,&stud[i].ID,&stud[i].score,cdept)!=true) cout<<"In Insert "<<i<<": Error\n";

    }

    ifs.close();

    cout<<"\nread Test ---------------\n";
    for(int i=0;i<10000;i++) {
        if(i%1000==0) {
            read(i,cname,&ID,&score,cdept);
            cout<<stud[i].name<<" "<<stud[i].ID<<" "<<stud[i].score<<" "<<stud[i].dept<<endl;
            cout<<cname<<" "<<ID<<" "<<score<<" "<<cdept<<endl;
        }
    }

    cout<<"\nsearch Test ---------------\n";
    for(int i=0;i<10000;i++) {
        if(i%1000==0) {
            if(search(cname, stud[i].ID, &score, cdept) != true) cout<<"In Search "<<i<<": Error\n";
            cout<<stud[i].name<<" "<<stud[i].ID<<" "<<stud[i].score<<" "<<stud[i].dept<<endl;
            cout<<cname<<" "<<stud[i].ID<<" "<<score<<" "<<cdept<<endl;
        }
    }

    cout<<"\ndrop Test ---------------\n";

    for(int i=0;i<10000;i++) {
        if(i%10==0) {
            strcpy(cname,stud[i].name.c_str());
            strcpy(cdept,stud[i].dept.c_str());
            drop(cname,stud[i].ID,&stud[i].score,cdept);

        }
    }

    for(int i=0;i<10000;i++) {
        if(i%1000==0) {
            if(search(cname, stud[i].ID, &score, cdept)!=true) cout<<i<<": Error\n";
            else cout<<i<<": "<<cname<<" "<<ID<<" "<<score<<" "<<cdept<<endl;
        }
    }

    cout<<"\n2nd search Test ---------------\n";

    for(int i=0;i<10000;i++) {
        if(i%10==0) {
            strcpy(cname,stud[i].name.c_str());
            strcpy(cdept,stud[i].dept.c_str());
            if(insert(cname,&stud[i].ID,&stud[i].score,cdept)!=true) cout<<"In Insert "<<i<<": Error\n";

        }

    }

    for(int i=0;i<10000;i++) {
        if(i%1000==0) {
            if(search(cname,stud[i].ID,&score,cdept)!=true) cout<<"In Search "<<i<<": Error\n";
            else {
                cout<<stud[i].name<<" "<<stud[i].ID<<" "<<stud[i].score<<" "<<stud[i].dept<<endl;
                cout<<cname<<" "<<stud[i].ID<<" "<<score<<" "<<cdept<<endl;
            }
        }
    }

    return 0;

}

/*
int main() {
	코드 설명
    class는 멤버변수로 name, ID, score, dept를 가지며 객체의 크기는
    40바이트 입니다. 한 번의 insert때 마다 40바이트씩 커짐을 알 수 
    있습니다. ls -l student.dat를 했을 때 insert 수 * 40 외에 +1
    바이트가 추가로 붙는 경우는 메타데이터 때문일 뿐 실제 데이터는 
    40 바이트의 블록 단위로 하나의 객체를 저장하고 있습니다.
    
    
    //4개 데이터 입력 
    //insert("user1", 1111, 11.1, "one");
    //insert("user2", 2222, 22.2, "two");
	//insert("user3", 3333, 33.3, "three");
    //insert("user4", 4444, 44.4, "four");

        
    // 중복된 데이터 입력 
    //insert("user3", 3333, 33.3, "three");
    
    // drop으로 특정 데이터 삭제 
    //drop("user2", 2222, 22.2, "two");

    // drop이 발생한 빈 공간에 데이터 입력 
    //insert("user5", 5555, 55.5, "five");

    // 추가적인 데이터 입력
    //insert("user6", 6666, 66.6, "six");   
    //insert("user7", 7777, 77.7, "seven");

    // student.dat의 파일에 입력된 레코드들을 순차적으로 읽음
    for(int i = 0; i < 4; i++) {
        read(i, "xxx", 1234, 12.34, "xxx");
    }
   
    
    
	return 0;
}
*/











