#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;
const long long N = 1LL * 1024 * 1024 * 1024 * 40; // 10000K个4KB是38GB

string path = "/home/linguangming/wyxm/YCSB-C/RBD/db.txt";
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<double> Duration;

int main() {
    Clock::time_point start = Clock::now();
    ofstream out(path);
    for (long long i = 0; i < N; i++) {
        out << "a";
    }
    out.close();
    Duration d = Clock::now() - start;
    cout << "Time: " << d.count() << "s" << endl;
    return 0;
}

/*
    本地盘
    运行时间 1075.35s
    带宽40GB / 1075.35s = 38.09MB/s
*/
/*
    ceph盘
    运行时间 1069.03s
    带宽40GB / 1069.03s = 38.32MB/s
*/