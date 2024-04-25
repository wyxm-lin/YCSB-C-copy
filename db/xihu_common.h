#include <stdio.h>

#include <iostream>

#include "/home/linguangming/wyxm/JusteRPC/eRPC/src/rpc.h"

static constexpr uint8_t kReqType = 2;

#define MSG_BUFFER_NUM 50
#define MAX_PKT_SIZE 3824
#define MAX_MSG_SIZE 8353520
#define SERVER_SIZE 0x100000000     // 4GB，每块4KB，共1M块
#define BLOCKS_PER_SERVER 0x100000  // 1M块
#define BLOCKSIZE (3824 - sizeof(CommonMsg))
static constexpr uint64_t PIPLINE_PKT_DEPTH = 30;

enum class MsgType : uint8_t { Read = 0, Write };  // 信息类型

// 通用信息传递结构体
struct CommonMsg {
  MsgType type;  // 信息类型
  union {
    struct {
      uint32_t bid;
    } Request;
  } u;
};

// 任务信息结构体
struct Mission {
  MsgType type;
  int64_t bid;
  int64_t nxt;
  int64_t tot;
  int64_t pip;
  uint64_t ptr;
};