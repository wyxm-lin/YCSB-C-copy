//
//  xihu_db.h
//  YCSB-C

#ifndef YCSB_C_XIHU_DB_H_
#define YCSB_C_XIHU_DB_H_

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

namespace ycsbc {

class XihuDB : public DB {
 public:
  XihuDB() {
    phase = INVALID;
    cout << "XiHu DB opened." << endl;
  }

  ~XihuDB() {
    cout << "XiHu DB closed." << endl;
  }

  void Init(bool flag) { 
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
    cout << "bandwidth: " << ops << " * " << "4KB / 1024 / " << AllDuration << " = " << 1.0 * ops * 4 / 1024 / AllDuration<< " MB/s" << endl;
    double LoadDuration = 0;
    for (auto i : LOAD_TIME) {
      LoadDuration += i;
    }
    cout << "LoadDuration: " << LoadDuration << endl;
    cout << "LoadSpeed: " << ops / LoadDuration / 1000 << " KTPS" << endl;
  }

  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields, // 这个地方会传参数进来
           std::vector<KVPair> &result) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer;
    timer.Start(); 

    int RealKey = std::stoi(key.substr(4));
    std::streamoff offset = RealKey * 4096;

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

    cout << "XIHU DB SCAN not implemented\n";

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
  enum Phase phase;
};

} // ycsbc

#endif // YCSB_C_CEPH_DB_H_
