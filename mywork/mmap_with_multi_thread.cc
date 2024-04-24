#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <future>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <mutex>
#include <map>

using namespace std;
string filepath = "/home/linguangming/wyxm/YCSB-C/mywork/test.txt";
const int RecordNumber = 64;
const int ops = 32;
const int threadnum = 4;
const int Size = 4;

map<int, char> record;

class DB {
public:
    int fd;
    void* addr;
    off_t filesize;
    std::mutex mtx[RecordNumber];

    DB() {}
    ~DB() {}

    void Init() {
        fd = open(filepath.c_str(), O_RDWR | O_CREAT, 0666);
        struct stat sb;
        fstat(fd, &sb);
        filesize = sb.st_size;
        addr = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        cout << "Init Success\n";
    }

    void Read(int off) {
        std::lock_guard<std::mutex> lock(mtx[off]);
        cout << "Read " << off << endl;
        char buffer[Size];
        char* src = static_cast<char*>(addr) + off * Size;
        memcpy(buffer, src, Size);
    }

    void Write(int off, char ch) {
        std::lock_guard<std::mutex> lock(mtx[off]);
        cout << "Write " << off << endl;
        char* src = static_cast<char*>(addr) + off * Size;
        memset(src, ch, Size);
        msync(src, Size, MS_SYNC);
        record[off] = ch;
    }
    
    void Close() {
        munmap(addr, filesize);
        close(fd);
        cout << "Close Success\n";
    }
};

void init() {
    ofstream out(filepath);
    for (int i = 0; i < RecordNumber * Size; i++) {
        out << "_";
    }
    out.close();
}

class Client {
public:
    DB* db_;

    Client() {}
    ~Client() {}
    void Init(DB* db) {
        db_ = db;
    }
};

void DelegateClient(DB* db, int num_ops) {
    Client client;
    client.Init(db);
    for (int i = 0; i < num_ops; i++) {
        int Type = rand() % 100;
        if (Type >= 50) { // 读
            int off = rand() % RecordNumber;
            client.db_->Read(off);
        }
        else { // 写
            int off = rand() % RecordNumber;
            char ch = 'a' + rand() % 26;
            client.db_->Write(off, ch);
        }
    }
}

void check() {
    bool flag = true;
    ifstream in(filepath);
    char ch;
    int cnt = 0;
    while (in >> ch) {
        int belong = cnt / Size;
        if (record.count(belong) == 0) {
            if (ch != '_') {
                cout << "Error: " << cnt << " " << record[belong] << " " << ch << endl;
                flag = false;
            }
        }
        else {
            if (record[belong] != ch) {
                cout << "Error: " << cnt << " " << record[belong] << " " << ch << endl;
                flag = false;
            }
        }   
        cnt ++;
    }  
}

int main() {
    init();

    DB* db = new DB();
    db->Init();

    vector<future<void>> actual_ops;
    for (int i = 0; i < threadnum; i++) {
        actual_ops.emplace_back(async(launch::async, DelegateClient, db, ops / threadnum));
    }
    assert((int)actual_ops.size() == threadnum);

    for (auto &n : actual_ops) {
        assert(n.valid());
        n.get();
    }

    db->Close();

    check();

    return 0;
}