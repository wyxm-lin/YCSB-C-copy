#include <iostream>
#include <chrono>
#include <queue>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <cstdint>

using namespace std;

const int mod = 10000000;
int ops = 10000000;

std::string ceph_db_path = "/home/linguangming/wyxm/YCSB-C/DBMS/simpledb.txt"; // comment 这个是数据库的路径


// FNV-1 32位哈希的基本参数
constexpr uint32_t kFNVOffsetBasis32 = 2166136261u;
constexpr uint32_t kFNVPrime32 = 16777619u;

// 32位FNV-1哈希函数
inline uint32_t FNVHash32(uint32_t val) {
    uint32_t hash = kFNVOffsetBasis32;

    for (int i = 0; i < 4; i++) {  // 只循环4次，每次处理一个字节
        uint32_t octet = val & 0xff;  // 提取最低字节
        val >>= 8;  // 移动到下一个字节

        hash ^= octet;  // XOR当前字节到哈希值
        hash *= kFNVPrime32;  // 乘以FNV素数
    }
    return hash;
}

fstream db_file;

void MyRead(int pos) {
    ifstream db_file;
    db_file.open(ceph_db_path.c_str(), std::ios::in | std::ios::out);
    if (!db_file.is_open()) {
        cerr << "open db file failed." << endl;
        exit(1);
    }
    db_file.seekg(pos);
    std::vector<char> buffer(4096);
    db_file.read(buffer.data(), 4096);
    db_file.close();
}

void MyWrite(int pos) {
    ofstream db_file;
    db_file.open(ceph_db_path.c_str(), std::ios::in | std::ios::out);
    if (!db_file.is_open()) {
        cerr << "open db file failed." << endl;
        exit(1);
    }
    db_file.seekp(pos);
    std::vector<char> data(4096, 'a'); // 4KB的'a'
    db_file.write(data.data(), data.size());
    db_file.flush();
    db_file.close();
}

int ReadRemain, WriteRemain;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<double> Duration;

int main() {
    cerr << "start\n";
    db_file.open(ceph_db_path.c_str(), std::ios::in | std::ios::out);
    if (!db_file.is_open()) {
        cerr << "open db file failed." << endl;
        exit(1);
    }
    ReadRemain = WriteRemain = ops / 2;
    Clock::time_point t = Clock::now();
    for (int i = 0; i < ops; i++) {
        int pos = rand() % mod;
        int Type = rand() % 100;
        if (ReadRemain && WriteRemain) {
            if (Type >= 50) { // read
                MyRead(pos);
                ReadRemain --;
            }
            else {
                MyWrite(pos);
                WriteRemain --;
            }
        }
        else if (ReadRemain) {
            MyRead(pos);
        }
        else if (WriteRemain) {
            MyWrite(pos);
        }
    }
    Duration span = std::chrono::duration_cast<Duration>(Clock::now() - t);
    cout << "Time: " << span.count() << "s" << endl;
    return 0;
}

/*
65s -> 600MB/s
72s -> 545MB/s
70s -> 555MB/s
*/
/*
29s -> 1.3GB/s
34s -> 1.1GB/s
28s -> 1.4GB/s
*/