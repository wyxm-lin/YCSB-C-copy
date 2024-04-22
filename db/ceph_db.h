//
//  ceph_db.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/17/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//
#define MMAP

#ifndef MMAP
#ifndef YCSB_C_CEPH_DB_H_
#define YCSB_C_CEPH_DB_H_

#include "core/db.h"

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <cmath>
#include "core/properties.h"
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "core/timer.h"

using std::cout;
using std::endl;
using std::vector;

// static string ceph_db_path = "../DBMS/simpledb.txt"; // comment 这个是数据库的路径

enum Phase {
  INVALID,
  LOAD,
  TRANSACTION
};

namespace ycsbc {
std::string ceph_db_path = "/home/linguangming/wyxm/YCSB-C/DBMS/simpledb.txt"; // comment 这个是数据库的路径
// std::string ceph_db_path = "/home/linguangming/wyxm/YCSB-C/RBD/simpledb.txt"; // comment 这个是数据库的路径

class CephDB : public DB {
 public:
  CephDB() { // comment 这个构造函数什么都没干
    phase = INVALID;
    db_file_.open(ceph_db_path.c_str(), std::ios::in | std::ios::out | std::ios::app);
    if (!db_file_.is_open()) {
      std::cerr << "open db file failed." << std::endl;
      exit(1);
    }
    cout << "db file opened." << endl; // comment 这个地方会打印 "db file opened."
  }

  ~CephDB() { // comment 这个析构函数什么都没干
    db_file_.close();
    cout << "db file closed." << endl; // comment 这个地方会打印 "db file closed."
  }

  void Init(bool flag) { // comment 这个init啥都没干
    std::lock_guard<std::mutex> lock(mutex_);
    if (flag == true) {
      phase = LOAD;
      cout << "A new thread begins loading." << endl;
    } else if (flag == false) {
      phase = TRANSACTION;
      cout << "A new thread begins transaction." << endl;
    }
  }

  void HandleAllData() {
    double ReadDuration = 0;
    double InsertDuration = 0;
    double UpdateDuration = 0;
    double DeleteDuration = 0;
    double ScanDuration = 0;
    for (auto i : READ_TIME) {
      ReadDuration += i;
    }
    for (auto i : INSERT_TIME) {
      InsertDuration += i;
    }
    for (auto i : UPDATE_TIME) {
      UpdateDuration += i;
    }
    for (auto i : DELETE_TIME) {
      DeleteDuration += i;
    }
    for (auto i : SCAN_TIME) {
      ScanDuration += i;
    }
    cout << "ReadDuration: " << ReadDuration << endl;
    cout << "InsertDuration: " << InsertDuration << endl;
    cout << "UpdateDuration: " << UpdateDuration << endl;
    cout << "DeleteDuration: " << DeleteDuration << endl;
    cout << "ScanDuration: " << ScanDuration << endl;
    cout << "Operation: " << ops << endl;
    double AllDuration = ReadDuration + InsertDuration + UpdateDuration + DeleteDuration + ScanDuration;
    cout << "AllDuration: " << AllDuration << endl;
    double speed = ops / AllDuration / 1000;
    cout << "speed: " << speed << " KTPS" << endl;
    cout << "bandwidth: " << ops << " * " << "4KB / 1024 / " << AllDuration << " = " << ops * 4 / 1024 / AllDuration<< " MB/s" << endl;
    double LoadDuration = 0;
    for (auto i : LOAD_TIME) {
      LoadDuration += i;
    }
    cout << "LoadDuration: " << LoadDuration << endl;
    cout << "LoadSpeed: " << ops / LoadDuration / 1000 << " KTPS" << endl;
  }

  void Flush() {
    if (ops % 10 == 0) {
      db_file_.flush();
      db_file_.close();
      db_file_.open(ceph_db_path.c_str(), std::ios::in | std::ios::out | std::ios::app);
    }
  }

  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields, // 这个地方会传参数进来
           std::vector<KVPair> &result) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer;
    timer.Start(); 

    int RealKey = std::stoi(key.substr(4));
    // cout << "Read: " << "RealKey: " << RealKey << " Key: " << key << endl; // comment 这个地方会打印 "Read: RealKey: 1 Key: user1

    std::streamoff offset = RealKey * 4096;
    db_file_.seekg(offset, std::ios::beg); // seekg -> read
    std::vector<char> buffer(4096);
    db_file_.read(buffer.data(), 4096);

    Flush();

    READ_TIME.push_back(timer.End());
    return 0;
  }

  int Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时

    cout << "Ceph DB SCAN not implemented\n";

    Flush();

    SCAN_TIME.push_back(timer.End());
    return 0;
  }

  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer;
    timer.Start();

    int RealKey = std::stoi(key.substr(4));

    std::streamoff offset = RealKey * 4096;
    db_file_.seekp(offset, std::ios::beg); // seekp -> write
    std::vector<char> data(4096, 'a'); // 4KB的'a'
    db_file_.write(data.data(), data.size());
    db_file_.flush();

    Flush();

    UPDATE_TIME.push_back(timer.End());
    return 0;
  }

  int Insert(const std::string &table, const std::string &key, // comment 原先该函数的实现 仅仅只是把key和value打印出来 没有实际存储起来
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (phase == TRANSACTION) {
      ops ++;
    }
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时

    int RealKey = std::stoi(key.substr(4));

    std::streamoff offset = RealKey * 4096;
    db_file_.seekp(offset, std::ios::beg); // seekp -> write
    std::vector<char> data(4096, 'b'); // 4KB的'b'
    db_file_.write(data.data(), data.size());
    db_file_.flush();

    Flush();

    if (phase == TRANSACTION) { // 只有transaction阶段才会记录时间
      INSERT_TIME.push_back(timer.End());
    }
    else {
      LOAD_TIME.push_back(timer.End());
    }
    return 0;
  }

  int Delete(const std::string &table, const std::string &key) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时

    cout << "DELETE " << table << ' ' << key << endl;

    Flush();

    DELETE_TIME.push_back(timer.End());
    return 0; 
  }

 private:
  std::mutex mutex_; // 
  std::fstream db_file_; // comment 这个是数据库文件
  long long RANGE = 1LL << 32; // comment 这个是一个常量
  vector<double> READ_TIME;
  vector<double> INSERT_TIME;
  vector<double> UPDATE_TIME;
  vector<double> DELETE_TIME;
  vector<double> SCAN_TIME;
  vector<double> LOAD_TIME;
  long long ops = 0;
  enum Phase phase;
};

} // ycsbc

#endif // YCSB_C_CEPH_DB_H_
#endif

//
//  basic_db.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/17/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//
#ifdef MMAP
#ifndef YCSB_C_CEPH_DB_H_
#define YCSB_C_CEPH_DB_H_

#include "core/db.h"

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <cmath>
#include "core/properties.h"
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include "core/timer.h"

using std::cout;
using std::endl;
using std::vector;

// static string ceph_db_path = "../DBMS/simpledb.txt"; // comment 这个是数据库的路径

enum Phase {
  INVALID,
  LOAD,
  TRANSACTION
};

namespace ycsbc {
// std::string ceph_db_path = "/home/linguangming/wyxm/YCSB-C/DBMS/db.txt"; // comment 这个是数据库的路径
std::string ceph_db_path = "/home/linguangming/wyxm/YCSB-C/RBD/db.txt"; // comment 这个是数据库的路径

class CephDB : public DB {
 public:
  CephDB() {
    phase = INVALID;
    fd = open(ceph_db_path.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
      cout << "open db file failed." << endl;
      exit(-1);
    }
    // 获取文件的大小
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      cout << "fstat db file failed." << endl;
      exit(-1);
    }
    addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
      cout << "mmap db file failed." << endl;
      exit(-1);
    }
    cout << "db file opened." << endl;
  }

  ~CephDB() { // comment 这个析构函数什么都没干
    if (close(fd) == -1) {
      cout << "close db file failed." << endl;
      exit(-1);
    }
    cout << "db file closed." << endl; // comment 这个地方会打印 "db file closed."
  }

  void Init(bool flag) { // comment 这个init啥都没干
    std::lock_guard<std::mutex> lock(mutex_);
    if (flag == true) {
      phase = LOAD;
      cout << "A new thread begins loading." << endl;
    } else if (flag == false) {
      phase = TRANSACTION;
      cout << "A new thread begins transaction." << endl;
    }
  }

  void HandleAllData() {
    cout << "===================================================================\n";
    double ReadDuration = 0;
    double InsertDuration = 0;
    double UpdateDuration = 0;
    double DeleteDuration = 0;
    double ScanDuration = 0;
    for (auto i : READ_TIME) {
      ReadDuration += i;
    }
    for (auto i : INSERT_TIME) {
      InsertDuration += i;
    }
    for (auto i : UPDATE_TIME) {
      UpdateDuration += i;
    }
    for (auto i : DELETE_TIME) {
      DeleteDuration += i;
    }
    for (auto i : SCAN_TIME) {
      ScanDuration += i;
    }
    cout << "ReadDuration: " << ReadDuration << endl;
    cout << "InsertDuration: " << InsertDuration << endl;
    cout << "UpdateDuration: " << UpdateDuration << endl;
    cout << "DeleteDuration: " << DeleteDuration << endl;
    cout << "ScanDuration: " << ScanDuration << endl;
    cout << "Operation: " << ops << endl;
    double AllDuration = ReadDuration + InsertDuration + UpdateDuration + DeleteDuration + ScanDuration;
    cout << "AllDuration: " << AllDuration << endl;
    double speed = ops / AllDuration / 1000;
    cout << "speed: " << speed << " KTPS" << endl;
    cout << "bandwidth: " << ops << " * " << "4KB / 1024 / " << AllDuration << " = " << ops * 4 / 1024 / AllDuration<< " MB/s" << endl;
    double LoadDuration = 0;
    for (auto i : LOAD_TIME) {
      LoadDuration += i;
    }
    cout << "LoadDuration: " << LoadDuration << endl;
    cout << "LoadSpeed: " << ops / LoadDuration / 1000 << " KTPS" << endl;
    cout << "===================================================================\n";
  }

  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields, // 这个地方会传参数进来
           std::vector<KVPair> &result) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer;
    timer.Start(); 

    int RealKey = std::stoi(key.substr(4));
    char* offset_addr = static_cast<char*>(addr) + 1LL * RealKey * 4096;
    char buffer[4096];
    memcpy(buffer, offset_addr, 4096);

    READ_TIME.push_back(timer.End());
    return 0;
  }

  int Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时

    cout << "Ceph DB SCAN not implemented\n";

    SCAN_TIME.push_back(timer.End());
    return 0;
  }

  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer;
    timer.Start();

    int RealKey = std::stoi(key.substr(4));
    char* offset_addr = static_cast<char*>(addr) + 1LL * RealKey * 4096;
    memset(offset_addr, 'a', 4096);

    UPDATE_TIME.push_back(timer.End());
    return 0;
  }

  int Insert(const std::string &table, const std::string &key, // comment 原先该函数的实现 仅仅只是把key和value打印出来 没有实际存储起来
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (phase == TRANSACTION) {
      ops ++;
    }
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时

    // cout << "begin" << endl;
    int RealKey = std::stoi(key.substr(4));
    char* offset_addr = static_cast<char*>(addr) + 1LL * RealKey * 4096;
    memset(offset_addr, 'b', 4096);
    // cout << "end" << endl;

    if (phase == TRANSACTION) { // 只有transaction阶段才会记录时间
      INSERT_TIME.push_back(timer.End());
    }
    else {
      LOAD_TIME.push_back(timer.End());
    }
    return 0;
  }

  int Delete(const std::string &table, const std::string &key) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时

    cout << "DELETE " << table << ' ' << key << endl;

    DELETE_TIME.push_back(timer.End());
    return 0; 
  }

 private:
  std::mutex mutex_; // 
  vector<double> READ_TIME;
  vector<double> INSERT_TIME;
  vector<double> UPDATE_TIME;
  vector<double> DELETE_TIME;
  vector<double> SCAN_TIME;
  vector<double> LOAD_TIME;
  long long ops = 0;
  int fd;
  void* addr;
  enum Phase phase;
};

} // ycsbc

#endif // YCSB_C_CEPH_DB_H_
#endif
