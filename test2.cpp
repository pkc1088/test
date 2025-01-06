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

bool search(char* name, unsigned ID, float score, char* dept) {
	ifstream file("example.dat", ios::binary);
    if(!file) return false; 
    cout << "search here\n";
    Student temp;
    while(file.read(reinterpret_cast<char*>(&temp), sizeof(Student))) {
        cout << "entered " << temp.ID << ", local ID : " << ID << endl;
        if(temp.ID == ID) {
            /*
            //strcpy(name, temp.name);
            //temp.score = score;
            //strcpy(dept, temp.dept);
            
            Student obj;
            obj.name = new char[20]; // 동적으로 메모리 할당
            strcpy(obj.name, name); // 문자열 복사
            */

            //file.close();
            cout << "found! " << temp.score << endl;
            file.close();
            return true;
        }
    }
    cout << "search out\n";
    file.close();
    return false;
}

bool insert(char* name, unsigned int ID, float score, char* dept) {
    ifstream file("example.dat", ios::binary);
    if(!file) {
        ofstream newFile("example.dat", ios::binary);
        Student record(name, ID, score, dept);
        cout << "first record entered : " << record.ID << endl;
        newFile.write(reinterpret_cast<const char*>(&record), sizeof(Student));
        return true;
    }

    if(search(name, ID, score, dept)) { cout << "duplicated!" << endl; return false; }
    
    Student real_record(name, ID, score, dept);
    Student record(name, ID, score, dept);
    //   
    file.close();
    fstream emfile("example.dat", ios::in | ios::out | ios::binary); 
    emfile.seekg(0, ios::beg); // Go to the beginning of the file
    while (emfile.read(reinterpret_cast<char*>(&record), sizeof(Student))) { 
        // 이 짓에서 record를 변화시키는게 문제였음
        cout << "record.ID : " << record.ID << endl;
        if (record.ID == 0) { // Found an empty record slot
            cout << "found empty" << endl;
            emfile.seekp(-static_cast<long>(sizeof(Student)), ios::cur); // Move back to the empty slot
            emfile.write(reinterpret_cast<char*>(&real_record), sizeof(Student)); // Write the new record
            emfile.close();
            return true;
        }
    }
    emfile.close();
    
    ofstream wRecord("example.dat", ios::binary | ios::app);;
    wRecord.write(reinterpret_cast<const char*>(&real_record), sizeof(Student));
	cout << "record saved at the end " << record.ID << endl;

    return true;
} 

/*
bool insert2(char* name, unsigned int ID, float score, char* dept) {
    fstream file("example.dat", ios::binary | ios::in | ios::out);
    if (!file) {
        ofstream newFile("example.dat", ios::binary);
        Student record(name, ID, score, dept);
        newFile.write(reinterpret_cast<const char*>(&record), sizeof(Student));
        return true;
    }

    if (search(name, ID, score, dept)) {
        cout << "duplicated!" << endl;
        return false;
    }
    
    Student record(name, ID, score, dept);
    bool foundEmptySlot = false;

    // Find empty record slot
    file.seekg(0, ios::beg); // Go to the beginning of the file
    while (file.read(reinterpret_cast<char*>(&record), sizeof(Student))) {
        if (record.ID == 0) { // Found an empty record slot
            cout << "found empty!" << endl;
            foundEmptySlot = true;
            break;
        }
    }

    if (foundEmptySlot) {
        file.seekp(-static_cast<long>(sizeof(Student)), ios::cur); // Move back to the empty slot
        file.write(reinterpret_cast<char*>(&record), sizeof(Student)); // Write the new record
    } else {
//        file.seekp(0, ios::end); // Move to the end of the file
//        file.write(reinterpret_cast<char*>(&record), sizeof(Student)); // Write the new record
        ofstream wRecord("example.dat", ios::binary | ios::app);;
        wRecord.write(reinterpret_cast<const char*>(&record), sizeof(Student));
    }

    return true;
}
*/

bool read(int n, char* name, unsigned int ID, float score, char* dept) {
    ifstream file("example.dat", ios::binary);
    if(!file) return false;

    Student temp;
    file.seekg(n * sizeof(Student), ios::beg); 
    
    if (file.fail()) { cout << "error1" << endl; file.close(); return false; }
    // 위치 이동에 실패했거나 파일 끝을 넘어선 경우
    
    file.read(reinterpret_cast<char*>(&temp), sizeof(Student));
   
    cout << "Name : " << temp.name << ", ID : " << temp.ID << 
            ", score : " << temp.score << ", dept : " << temp.dept << endl;

    if (file.gcount() != sizeof(Student)) { cout << "error2" << endl; file.close(); return false; }
    // 요청한 만큼 데이터를 읽지 못했을 경우 (파일 끝에 도달했거나 데이터가 부족한 경우)

    file.close();
	return true;
}

bool drop(char* name, unsigned ID, float score, char* dept) {
    fstream file("example.dat", ios::binary | ios::in | ios::out);
    if(!file) return false;    

    Student temp;
    while(file.read(reinterpret_cast<char*>(&temp), sizeof(Student))) {
        cout << "drop entered " << temp.ID << endl;
        if(temp.ID == ID) {
            temp.ID = 0;
            strcpy(temp.name, "");
            temp.score = 0.0;
            strcpy(temp.dept, "");

            file.seekp(-static_cast<long>(sizeof(Student)), ios::cur); // 현재 위치로부터 한 레코드 크기 전으로 이동
            file.write(reinterpret_cast<char*>(&temp), sizeof(Student)); // 변경된 레코드를 다시 파일에 씀            
            file.close();
            return true;
        }
    }

	return true;
}

int main() {
	
    /*
    insert("pyeon", 1234, 99.9, "cse");
    insert("kim", 5678, 92.9, "phi");
	insert("park", 9125, 75.5, "ele");
    insert("choi", 1088, 77.7, "mac");
    */
    
    //drop("pyeon", 1234, 92.9, "xcse");
    //insert("chan", 9898, 22.2, "PE");
    //insert("Kwon", 2424, 24.4, "art");   
    //insert("John", 1111, 11.11, "mma");

    insert("Kwon", 2424, 24.4, "art");
    for(int i = 0; i < 6; i++) {
        read(i, "xxx", 111, 11.5, "xxx");
    }

    
    
	return 0;
}












