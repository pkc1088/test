#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

const int MaxBlockSize = 4096;
const int RecordSize = 38;
const unsigned char isNotUsing = 0x0;
const unsigned char isPadding = "\xff"[0];

bool insert(char *name, unsigned *ID, float *score, char *dept)
{
    int NowDataSize;
    char writeBuf[38];
    copy(name, name + 20, writeBuf);
    copy((char *)ID, (char *)ID + 4, writeBuf + 20);
    copy((char *)score, (char *)score + 4, writeBuf + 24);
    copy(dept, dept + 10, writeBuf + 28);
    int FindBlock = 0;
    try
    {
        fstream isopen("Student.dat", ios::in | ios::out | ios::binary);
        if (!isopen.is_open())
        {
            ofstream makefile("Student.dat", ios::out | ios::binary);
            makefile.write(writeBuf, 38);
            makefile.close();
            makefile.close();
            isopen.close();
            return true;
        }
        fstream fbinout("Student.dat", ios::in | ios::out | ios::binary);
        ifstream fbin;
        fbin.open("Student.dat", ios::in | ios::binary);
        char buf[MaxBlockSize];
        fbin.read(buf, MaxBlockSize);
        NowDataSize = fbin.gcount();
        while (NowDataSize >= MaxBlockSize)
        {
            for (int i = 0; i < NowDataSize; i = i + 38)
            {

                if (*(buf + i) == isNotUsing)
                {
                    fbinout.seekp(FindBlock * MaxBlockSize + i, ios_base::beg);
                    fbinout.write(writeBuf, 38);
                    fbinout.close();
                    fbin.close();
                    return true;
                }
            }
            fbin.read(buf, MaxBlockSize);
            NowDataSize = fbin.gcount();
            FindBlock += 1;
        }
        if (MaxBlockSize - RecordSize <= NowDataSize)
        {
            for (int j = NowDataSize; j < MaxBlockSize; j += 1)
            {
                fbinout.seekp(0, ios_base::end);
                fbinout.write("\xff", 1);
            }
            fbin.read(buf, MaxBlockSize);
            NowDataSize = fbin.gcount();
            FindBlock += 1;
        }
        for (int i = 0; i < NowDataSize; i = i + 38)
        {
            if (*(buf + i) == isNotUsing)
            {
                fbinout.seekp(FindBlock * MaxBlockSize + i, ios_base::beg);
                fbinout.write(writeBuf, 38);
                fbinout.close();
                fbin.close();
                return true;
            }
        }
        fbinout.seekp(0, ios_base::end);
        fbinout.write(writeBuf, 38);
        fbinout.close();
        fbin.close();
        return true;
    }
    catch (int exp)
    {
        return false;
    }
};

bool read(int n, char *name, unsigned *ID, float *score, char *dept)
{
    int NowDataSize;
    ifstream fbin;
    fbin.open("Student.dat", ios::in | ios::binary);
    char buf[MaxBlockSize];
    fbin.read(buf, MaxBlockSize);
    NowDataSize = fbin.gcount();
    while (NowDataSize >= MaxBlockSize)
    {
        for (int i = 0; i < NowDataSize; i = i + 38)
        {
            if (*(buf + i) == isNotUsing || (strcmp((buf + i), "\xff")) == 1)
            {
                continue;
            }
            else if (n == 0)
            {
                copy(buf + i, buf + i + 20, name);
                *ID = *(unsigned *)(buf + i + 20);
                *score = *(float *)(buf + i + 24);
                copy(buf + i + 28, buf + i + 38, dept);
                fbin.close();
                return true;
            }
            else if (*(buf + i) != isNotUsing && (strcmp((buf + i), "\xff")) != 1)
            {
                n -= 1;
            }
        }
        fbin.read(buf, MaxBlockSize);
        NowDataSize = fbin.gcount();
    }
    for (int i = 0; i < NowDataSize; i = i + 38)
    {
        if (*(buf + i) == isNotUsing || (strcmp((buf + i), "\xff")) == 1)
        {
            continue;
        }
        else if (n == 0)
        {
            copy(buf + i, buf + i + 20, name);
            *ID = *(unsigned *)(buf + i + 20);
            *score = *(float *)(buf + i + 24);
            copy(buf + i + 28, buf + i + 38, dept);
            fbin.close();
            return true;
        }
        else if (*(buf + i) != isNotUsing && (strcmp((buf + i), "\xff")) != 1)
        {
            n -= 1;
        }
    }
    fbin.close();
    return false;
};

bool search(char *name, unsigned ID, float *score, char *dept)
{
    int NowDataSize;
    ifstream fbin;
    fbin.open("Student.dat", ios::in | ios::binary);
    char buf[MaxBlockSize];
    fbin.read(buf, MaxBlockSize);
    NowDataSize = fbin.gcount();
    while (NowDataSize >= MaxBlockSize)
    {
        for (int i = 0; i < NowDataSize; i = i + 38)
        {
            if (*(unsigned *)(buf + i + 20) == ID && (strcmp((buf + i), "\xff")) != 1 && *(buf + i) != isNotUsing)
            {
                copy(buf + i, buf + i + 20, name);
                *score = *(float *)(buf + i + 24);
                copy(buf + i + 28, buf + i + 38, dept);
                fbin.close();
                return true;
            }
        }
        fbin.read(buf, MaxBlockSize);
        NowDataSize = fbin.gcount();
    }
    for (int i = 0; i < NowDataSize; i = i + 38)
    {
        if (*(unsigned *)(buf + i + 20) == ID && (strcmp((buf + i), "\xff")) != 1 && *(buf + i) != isNotUsing)
        {
            copy(buf + i, buf + i + 20, name);
            *score = *(float *)(buf + i + 24);
            copy(buf + i + 28, buf + i + 38, dept);
            fbin.close();
            return true;
        }
    }
    fbin.close();
    return false;
};

bool drop(char *name, unsigned ID, float *score, char *dept)
{
    int NowDataSize;
    int FindBlock = 0;
    char nullBuf[38] = {
        0,
    };
    ifstream fbin;
    fbin.open("Student.dat", ios::in | ios::binary);
    fstream fbinout("Student.dat", ios::in | ios_base::out | ios_base::binary);
    char buf[MaxBlockSize];
    fbin.read(buf, MaxBlockSize);
    NowDataSize = fbin.gcount();
    while (NowDataSize >= MaxBlockSize)
    {
        for (int i = 0; i < NowDataSize; i = i + 38)
        {
            if (*(unsigned *)(buf + i + 20) == ID && (strcmp((buf + i), "\xff")) != 1 && *(buf + i) != isNotUsing)
            {
                fbinout.seekp(FindBlock * MaxBlockSize + i, ios_base::beg);
                fbinout.write(nullBuf, 38);
                fbinout.close();
                fbin.close();
                return true;
            }
        }
        fbin.read(buf, MaxBlockSize);
        NowDataSize = fbin.gcount();
        FindBlock += 1;
    }
    for (int i = 0; i < NowDataSize; i = i + 38)
    {
        if (*(unsigned *)(buf + i + 20) == ID && (strcmp((buf + i), "\xff")) != 1 && *(buf + i) != isNotUsing)
        {
            fbinout.seekp(FindBlock * MaxBlockSize + i, ios_base::beg);
            fbinout.write(nullBuf, 38);
            fbinout.close();
            fbin.close();
            return true;
        }
    }
    fbin.close();
    return false;
};

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
            if(search(cname,stud[i].ID,&score,cdept)!=true) cout<<"In Search "<<i<<": Error\n";
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
            if(search(cname,stud[i].ID,&score,cdept)!=true) cout<<i<<": Error\n";
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
