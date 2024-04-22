//
//  basic_db.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/17/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

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
#include "core/timer.h"

using std::cout;
using std::endl;
using std::vector;

// static string ceph_db_path = "../DBMS/simpledb.txt"; // comment 这个是数据库的路径

namespace ycsbc {
std::string ceph_db_path = "/home/linguangming/wyxm/YCSB-C/DBMS/simpledb.txt"; // comment 这个是数据库的路径

class CephDB : public DB {
 public:
  CephDB() { // comment 这个构造函数什么都没干
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

  void Init() { // comment 这个init啥都没干
    std::lock_guard<std::mutex> lock(mutex_);
    cout << "A new thread begins working." << endl;
  }

  int ParseKey(const std::string &key) { // NOTE: 此处需要消耗额外的时间
    int res = 0;
    cout << "ParseKey enter" << endl;
    for (size_t i = 0; i < key.size(); i++) {
      if ('0' <= key[i] && key[i] <= '9') {
         res = (1LL * res * 10 + key[i] - '0') % RANGE;
      }
    }
    res = abs(res);
    cout << "ParseKey exit" << endl;
    return res;
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
    double AllDuration = ReadDuration + InsertDuration + UpdateDuration + DeleteDuration + ScanDuration;
    cout << "AllDuration: " << AllDuration << endl;
    double speed = ops / AllDuration / 1000;
    cout << "speed: " << speed << " KTPS" << endl;
  }

  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields, // 这个地方会传参数进来
           std::vector<KVPair> &result) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时
    cout << "READ " << table << ' ' << key << endl;
    int RealKey = std::stoi(key.substr(4));
    cout << "RealKey: " << RealKey << endl;
    std::streamoff offset = RealKey * 1024; // Fix: Replace std::streamops with std::streamoff
    db_file_.seekg(offset, std::ios::beg); // std::ios::beg就是文件的开始位置
    std::vector<char> buffer(4096);
    db_file_.read(buffer.data(), 4096);
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
    // cout << "SCAN " << table << ' ' << key << " " << len;
    // if (fields) {
    //   cout << " [ ";
    //   for (auto f : *fields) {
    //     cout << f << ' ';
    //   }
    //   cout << ']' << endl;
    // } else {
    //   cout  << " < all fields >" << endl;
    // }
    SCAN_TIME.push_back(timer.End());
    return 0;
  }

  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时
    cout << "UPDATE " << table << ' ' << key << " [ ";
    for (auto v : values) {
      cout << v.first << '=' << v.second << ' ';
    }
    cout << ']' << endl;
    UPDATE_TIME.push_back(timer.End());
    return 0;
  }

  int Insert(const std::string &table, const std::string &key, // comment 原先该函数的实现 仅仅只是把key和value打印出来 没有实际存储起来
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时
    cout << "Ceph DB INSERT not implemented\n";
    // cout << "INSERT " << table << ' ' << key << " [ ";
    // for (auto v : values) {
    //   cout << v.first << '=' << v.second << ' ';
    // }
    // cout << ']' << endl;
    INSERT_TIME.push_back(timer.End());
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
  std::fstream db_file_; // comment 这个是数据库文件
  long long RANGE = 1LL << 32; // comment 这个是一个常量
  vector<double> READ_TIME;
  vector<double> INSERT_TIME;
  vector<double> UPDATE_TIME;
  vector<double> DELETE_TIME;
  vector<double> SCAN_TIME;
  long long ops = 0;
};

} // ycsbc

#endif // YCSB_C_CEPH_DB_H_

