//
//  real_ceph_db.h
//  YCSB-C
//

// MSYNC 是同步落盘
// MASYNC 是异步落盘


#ifndef YCSB_C_REAL_CEPH_DB_H_
#define YCSB_C_REAL_CEPH_DB_H_

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
#include "/usr/include/rados/librados.h"
#include "/usr/include/rbd/librbd.h"
#include <atomic>
#include <condition_variable>

using std::cout;
using std::endl;
using std::cerr;
using std::vector;

std::atomic<int> async_op_count{0};
std::mutex async_op_mutex;
std::condition_variable async_op_cv;

void aio_completion_callback(rbd_completion_t cb, void *arg) {
    char* buf = (char*)arg;
        // Check if the read operation was successful
    int ret = rbd_aio_get_return_value(cb);
    if (ret < 0) {
        // cerr << "Couldn't read object! error " << ret << std::endl;
    } else {
        // cout << "Read object completed successfully." << endl;
    }

    // Release the completion
    rbd_aio_release(cb);

    // Free the buffer
    free(buf);
    // cout << "Buffer freed." << endl;
    async_op_count--;
    async_op_cv.notify_all();
}

void aio_completion_write_callback(rbd_completion_t cb, void *arg) {
    int ret = rbd_aio_get_return_value(cb);
    if (ret < 0) {
      exit(1);
    } else {
    }

    rbd_aio_release(cb);

    async_op_count--;
    async_op_cv.notify_all();
}

void wait_for_all_async_ops() {
    std::unique_lock<std::mutex> lock(async_op_mutex);
    async_op_cv.wait(lock, []{ return async_op_count == 0; });
}

namespace ycsbc {

class RealCephDB : public DB {
 public:
  RealCephDB() {

    {
      std::cout << "High resolution clock period: "
              << std::chrono::high_resolution_clock::period::num
              << "/"
              << std::chrono::high_resolution_clock::period::den
              << " seconds" << std::endl;
    }

    phase = INVALID;
    const char *clustername = "b179329c-01e1-11ef-a63b-b7b87dd43c5a";
    
    int err = rados_create(&cluster, NULL);
    if (err < 0) {
        cerr << "Couldn't create the cluster handle! error " << err << std::endl;
        exit(1);
    }
    else {
        cout << "Created a cluster handle." << std::endl;
    }   

    err = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
    if (err < 0) {
        cerr << "Cannot read config file: " << err << std::endl;
        exit(1);
    }   
    else {
        cout << "Read the config file." << std::endl;
    }

    err = rados_connect(cluster);
    if (err < 0) {  
        cerr << "Cannot connect to cluster: " << err << std::endl;
        exit(1);
    }
    else {
        cout << "Connected to the cluster." << std::endl;
    }

    const char *poolname = "MyTest";
    err = rados_ioctx_create(cluster, poolname, &io);
    if (err < 0) {
        cerr << "Cannot open rados pool: " << err << std::endl;
        rados_shutdown(cluster);
        exit(1);
    }
    else {
        cout << "Created I/O context." << std::endl;
    }

    const char *imagename = "ceph-rbd-image1.img";

    err = rbd_open(io, imagename, &image, NULL);
    if (err < 0) {
        cerr << "Cannot open rbd image: " << err << std::endl;
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        exit(1);
    }
    else {
        cout << "Opened rbd image " << imagename << "." << std::endl;
    }

    cout << "db file opened." << endl;
  }

  void Close() {
    auto start = std::chrono::high_resolution_clock::now();
    wait_for_all_async_ops();
    auto end = std::chrono::high_resolution_clock::now();
    CLOSE_TIME = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    rbd_close(image);
    rados_ioctx_destroy(io);
    rados_shutdown(cluster);
    cout << "close OK\n";
  }

  ~RealCephDB() {
    cout << "db file closed." << endl;
  }

  void Init(bool flag) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (flag == true) {
      phase = LOAD;
      cout << "A new thread begins loading." << endl;
    } else if (flag == false) {
      phase = TRANSACTION;
      cout << "NO error\n";
      cout << "A new thread begins transaction.\n";
      cout << "No error\n";
    }
  }

  void HandleAllData() {
    cout << "===================================================================\n";
    double ReadDuration = 0;
    double InsertDuration = 0;
    double UpdateDuration = 0;
    double DeleteDuration = 0;
    double ScanDuration = 0;
    double FlushDuration = 0;
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
    for (auto i : FLUSH_TIME) {
      FlushDuration += i;
    }
    cout << "ReadDuration: " << ReadDuration << endl;
    cout << "InsertDuration: " << InsertDuration << endl;
    cout << "UpdateDuration: " << UpdateDuration << endl;
    cout << "DeleteDuration: " << DeleteDuration << endl;
    cout << "ScanDuration: " << ScanDuration << endl;
    cout << "FlushDuration: " << FlushDuration << endl;
    cout << "Operation: " << ops << endl;
    double AllDuration = ReadDuration + InsertDuration + UpdateDuration + DeleteDuration + ScanDuration + FlushDuration;
    cout << "AllDuration: " << AllDuration << endl;
    double speed = ops / AllDuration / 1000;
    cout << "speed: " << speed << " KTPS" << endl;
    cout << "bandwidth: " << ops << " * " << "4KB / 1024 / " << AllDuration << " = " << 1.0 * ops * 4 / 1024 / AllDuration<< " MB/s" << endl;
    cout << "===================================================================\n";
    cout << "CLOSE_TIME: " << CLOSE_TIME << "ns" << endl;
  }

  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields, // 这个地方会传参数进来
           std::vector<KVPair> &result) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer;
    timer.Start(); 

    char* buff = (char*)malloc(BlockSize);
    long long RealKey = std::stoi(key.substr(4));
    long long offset = 1LL * RealKey * BlockSize;
    rbd_completion_t comp;
    rbd_aio_create_completion(buff, aio_completion_callback, &comp);
    rbd_aio_read(image, offset, BlockSize, buff, comp);
    async_op_count++;

    READ_TIME.push_back(timer.End());
    return 0;
  }

  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    ops ++;
    utils::Timer<double> timer;
    timer.Start();

    char buf[BlockSize];
    memset(buf, 'a', BlockSize); // 注意此处需要占用写4096的时间
    buf[BlockSize - 1] = '\0';
    int RealKey = std::stoi(key.substr(4));
    long long offset = 1LL * RealKey * BlockSize;
    rbd_completion_t comp;
    rbd_aio_create_completion(NULL, aio_completion_write_callback, &comp);
    rbd_aio_write(image, offset, BlockSize, buf, comp);
    async_op_count++;

    UPDATE_TIME.push_back(timer.End());
    return 0;
  }

  int Insert(const std::string &table, const std::string &key, // comment 原先该函数的实现 仅仅只是把key和value打印出来 没有实际存储起来
             std::vector<KVPair> &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (phase == INVALID || phase == LOAD) {
      return 0;
    }
    if (phase == TRANSACTION) {
      ops ++;
    }
    utils::Timer<double> timer; // 计时器
    timer.Start(); // 计时器开始计时

    char buf[BlockSize];
    memset(buf, 'a', BlockSize); // 注意此处需要占用写4096的时间
    buf[BlockSize - 1] = '\0';
    int RealKey = std::stoi(key.substr(4));
    long long offset = 1LL * RealKey * BlockSize;
    rbd_completion_t comp;
    rbd_aio_create_completion(NULL, aio_completion_write_callback, &comp);
    rbd_aio_write(image, offset, BlockSize, buf, comp);
    async_op_count++;

    if (phase == TRANSACTION) { // 只有transaction阶段才会记录时间
      INSERT_TIME.push_back(timer.End());
    }
    else {
      LOAD_TIME.push_back(timer.End());
    }
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
  std::mutex mutex_; 
  vector<double> READ_TIME;
  vector<double> INSERT_TIME;
  vector<double> UPDATE_TIME;
  vector<double> DELETE_TIME;
  vector<double> SCAN_TIME;
  vector<double> LOAD_TIME;
  vector<double> FLUSH_TIME;
  vector<double> SYNC_TIME;
  double CLOSE_TIME;
  long long ops = 0;
  rados_t cluster;
  rados_ioctx_t io;
  rbd_image_t image;
  const int BlockSize = 4096;
  enum Phase phase;

};

} // ycsbc

#endif // YCSB_C_REAL_CEPH_DB_H_

