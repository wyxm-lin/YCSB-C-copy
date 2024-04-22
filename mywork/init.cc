// #include <iostream>
// #include <fstream>
// #include <string>

// using namespace std;

// string ceph_db_path = "/home/linguangming/wyxm/YCSB-C/RBD/simpledb.txt"; // comment 这个是数据库的路径

// const long long Len = 1LL * 1024 * 1024 * 1024 * 16;


// int main() {
//     ofstream db_file;
//     db_file.open(ceph_db_path.c_str(), std::ios::in | std::ios::out);
//     if (!db_file.is_open()) {
//         cerr << "open db file failed." << endl;
//         exit(1);
//     }
//     cout << "db file opened." << endl;
//     for (long long i = 0; i < Len; i++) {
//         db_file << "a";
//     }
//     db_file.flush();
//     cout << "db file written." << endl;
//     db_file.close();
//     return 0;
// }