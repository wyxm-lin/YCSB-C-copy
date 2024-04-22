#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>

using namespace std;

string filepath = "/home/linguangming/wyxm/YCSB-C/DBMS/a.txt";

int main() {
    cout << "start\n";
    // 打开文件
    int fd = open(filepath.c_str(), O_RDWR);  // 修改这里
    if (fd == -1) {
        // 处理错误
    }

    // 获取文件的大小
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        // 处理错误
    }

    // 将文件映射到内存
    void* addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // 修改这里
    if (addr == MAP_FAILED) {
        // 处理错误
    }

    // 现在你可以像操作内存一样操作文件
    // addr指向的内存区域包含了文件的内容

    // // 计算偏移位置的地址
    // char* offset_addr = static_cast<char*>(addr) + 4096;

    // // 写入数据
    // memset(offset_addr, 'a', 4096);

    // 当你完成操作后，需要解除映射
    if (munmap(addr, sb.st_size) == -1) {
        // 处理错误
    }

    // 关闭文件
    if (close(fd) == -1) {
        // 处理错误
    }
    cout << "over\n";
    return 0;
}