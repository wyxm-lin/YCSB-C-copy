/*
    build: g++ -o main rbd.cc -lrbd -lrados -std=c++11
    run: ./main > print.txt
*/

#include <iostream>
#include "/usr/include/rados/librados.h"
#include "/usr/include/rbd/librbd.h"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace std;

chrono::duration<double> total_time(0);
vector<chrono::duration<double>> time_list;

char buf[4096];

void init() {
    memset(buf, 'a', 4096);
    buf[4095] = '\0';
}

void easy_write(rbd_image_t image) {
    int offset = rand() / 4096 * 4096;
    rbd_completion_t c;
    rbd_aio_create_completion(NULL, NULL, &c);
    // rbd_write(image, offset, 4096, buf); // 同步阻塞
    rbd_aio_write(image, offset, 4096, buf, c); // 异步非阻塞
}

void easy_read(rbd_image_t image) {
    char buf[4096];
    long long offset = rand() * 4096;
    rbd_read(image, offset, 4096, buf); // 同步阻塞
    cout << "read OK" << endl;
}

ssize_t io_write(rbd_image_t image) {
    int offset = rand() / 4096 * 4096;
    rbd_completion_t c;
    rbd_aio_create_completion(NULL, NULL, &c);
    ssize_t ret = 0;
    auto start = chrono::high_resolution_clock::now();
    ret = rbd_write(image, offset, 4096, buf); // 同步阻塞
    // rbd_aio_write(image, offset, 4096, buf, c); // 异步非阻塞
    auto end = chrono::high_resolution_clock::now();
    // total_time += chrono::duration_cast<chrono::duration<double>>(end - start);
    // time_list.push_back(chrono::duration_cast<chrono::duration<double>>(end - start));
    
    if (ret < 0) {
        cerr << "write to image failed " << ret << std::endl;
        return ret;
    }
    else {
        // cout << "write to image success " << ret << std::endl;
    }
    return ret;
}

int main() {
    init();

    rados_t cluster;
    const char *clustername = "b179329c-01e1-11ef-a63b-b7b87dd43c5a";
    const char *name = NULL;
    uint64_t flags = 0;

    int err = rados_create(&cluster, NULL);
    if (err < 0) {
        cerr << "Couldn't create the cluster handle! error " << err << std::endl;
        return 1;
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

    rados_ioctx_t io;
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

    rbd_image_t image;
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

    {
        // transaction
        // for (int _ = 1; _ <= 1024; _ ++) {
        //     ssize_t ret = io_write(image);
        //     if (ret < 0) {
        //         cerr << "Cannot write to rbd image: " << ret << std::endl;
        //         rbd_close(image);
        //         rados_ioctx_destroy(io);
        //         rados_shutdown(cluster);
        //         exit(1);
        //     }
        // }
            
        auto start = chrono::high_resolution_clock::now();
        // for (int _ = 1; _ <= 1024; _ ++) {
        //     io_write(image);
        // }
        for (int _ = 1; _ <= 1024; _ ++) {
            easy_read(image);
        }
        auto end = chrono::high_resolution_clock::now();
        auto total = chrono::duration_cast<chrono::duration<double>>(end - start);
        start = chrono::high_resolution_clock::now();
        rbd_flush(image);
        end = chrono::high_resolution_clock::now();
        cout << "Flush time: " << chrono::duration_cast<chrono::duration<double>>(end - start).count() << "s" << endl;
        total += chrono::duration_cast<chrono::duration<double>>(end - start);
        cout << "Total time: " << total.count() << "s" << endl;
        cout << "throughput: " << 1024 * 4096 / total.count() / 1024 / 1024 << "MB/s" << endl;
    }

    

    err = rbd_close(image);
    if (err < 0) {
        cerr << "Cannot close rbd image: " << err << std::endl;
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
        exit(1);
    }
    else {
        cout << "Closed rbd image " << imagename << "." << std::endl;
    }

    rados_ioctx_destroy(io);
    rados_shutdown(cluster);

    for (auto it : time_list) {
        cout << it.count() << "s" << endl;
    }
    // cout << "Total time: " << total_time.count() << "s" << endl;
    // cout << "throughput: " << 1024 * 4096 / total_time.count() / 1024 / 1024 << "MB/s" << endl;
    return 0;
}
