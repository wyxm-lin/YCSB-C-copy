#include <queue>
#include <iostream>

#include "db/xihu_common.h"

Mission ms;
uint32_t session;
static erpc::Rpc<erpc::CTransport> *rpc;
static erpc::MsgBuffer reqs[MSG_BUFFER_NUM];
static erpc::MsgBuffer resps[MSG_BUFFER_NUM];
static std::queue<int64_t> avail;

using std::cout;
class XihuClient {
 private:
  erpc::Nexus nexus;

 public:
  XihuClient();
  ~XihuClient();

  int read(uint64_t, uint64_t, void *);
  int write(uint64_t, uint64_t, void *);
};

void cont_func(void *, void *cbctx) {
  avail.push((int64_t)cbctx);
  uint32_t seq = ms.nxt++;

  if (ms.type == MsgType::Write) {
    if (seq >= ms.tot) return;
    int64_t k = avail.front();
    avail.pop();
    erpc::MsgBuffer &req = reqs[k];
    erpc::MsgBuffer &resp = resps[k];
    CommonMsg *sdmsg = reinterpret_cast<CommonMsg *>(req.buf_);
    sdmsg->type = MsgType::Write;
    sdmsg->u.Request.bid = ms.bid + seq;
    memcpy(sdmsg + 1, (void *)(ms.ptr + seq * BLOCKSIZE), BLOCKSIZE);
    rpc->resize_msg_buffer(&req, sizeof(CommonMsg) + BLOCKSIZE);
    rpc->enqueue_request(session, kReqType, &req, &resp, cont_func, (void *)k);
  }

  else if (ms.type == MsgType::Read) {
    CommonMsg *rcvmsg = (CommonMsg *)resps[(int64_t)cbctx].buf_;
    void *dst = (void *)(ms.ptr + (rcvmsg->u.Request.bid - ms.bid) * BLOCKSIZE);
    memcpy(dst, rcvmsg + 1, BLOCKSIZE);
    if (seq >= ms.tot) return;
    int64_t k = avail.front();
    avail.pop();
    erpc::MsgBuffer &req = reqs[k];
    erpc::MsgBuffer &resp = resps[k];
    CommonMsg *sdmsg = reinterpret_cast<CommonMsg *>(req.buf_);
    sdmsg->type = MsgType::Read;
    sdmsg->u.Request.bid = ms.bid + seq;
    rpc->resize_msg_buffer(&req, sizeof(CommonMsg));
    rpc->enqueue_request(session, kReqType, &req, &resp, cont_func, (void *)k);
  }

  else
    exit(-1);
}

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void *) {}

void req_handler(erpc::ReqHandle *req_handle, void *) {
  auto &respmsg = req_handle->pre_resp_msgbuf_;
  rpc->enqueue_response(req_handle, &respmsg);
}

XihuClient::XihuClient() : nexus("10.10.10.13:31860") {
  cout << "XihuClient init\n";
  nexus.register_req_func(kReqType, req_handler);
  rpc = new erpc::Rpc<erpc::CTransport>(&nexus, nullptr, 0, sm_handler);
  for (int i = 0; i < MSG_BUFFER_NUM; i++) {
    avail.push(i);
    reqs[i] = rpc->alloc_msg_buffer_or_die(MAX_MSG_SIZE);
    resps[i] = rpc->alloc_msg_buffer_or_die(MAX_MSG_SIZE);
  }
  session = rpc->create_session("10.10.10.13:31861", 0);
  while (!rpc->is_connected(session)) rpc->run_event_loop_once();
  cout << "XihuClient init end\n";
}

XihuClient::~XihuClient() {
  delete rpc;
  rpc = nullptr;
}

int XihuClient::write(uint64_t blockID, uint64_t len, void *srcBuf) {
  ms.type = MsgType::Write;
  ms.tot = len;
  ms.bid = blockID;
  ms.ptr = (uint64_t)srcBuf;
  ms.pip = ms.nxt = std::min(PIPLINE_PKT_DEPTH, len);

  for (int i = 0; i < ms.nxt; i++) {
    int64_t k = avail.front();
    avail.pop();
    erpc::MsgBuffer &req = reqs[k];
    erpc::MsgBuffer &resp = resps[k];
    CommonMsg *msg = reinterpret_cast<CommonMsg *>(req.buf_);
    msg->type = MsgType::Write;
    msg->u.Request.bid = blockID + i;
    memcpy(msg + 1, (void *)((uint64_t)srcBuf + i * BLOCKSIZE), BLOCKSIZE);
    rpc->resize_msg_buffer(&req, sizeof(CommonMsg) + BLOCKSIZE);
    rpc->enqueue_request(session, kReqType, &req, &resp, cont_func, (void *)k);
  }

  // 数据请求仍实现为阻塞式
  while (ms.nxt < ms.tot + ms.pip) rpc->run_event_loop_once();
  return 0;
}

int XihuClient::read(uint64_t blockID, uint64_t len, void *dstBuf) {
  ms.type = MsgType::Read;
  ms.tot = len;
  ms.bid = blockID;
  ms.ptr = (uint64_t)dstBuf;
  ms.pip = ms.nxt = std::min(PIPLINE_PKT_DEPTH, len);
  cout << "read begin to loop\n";
  for (int i = 0; i < ms.nxt; i++) {
    int64_t k = avail.front();
    avail.pop();
    erpc::MsgBuffer &req = reqs[k];
    erpc::MsgBuffer &resp = resps[k];
    CommonMsg *msg = reinterpret_cast<CommonMsg *>(req.buf_);
    msg->type = MsgType::Read;
    msg->u.Request.bid = blockID + i;
    cout << "xihu read begin to resize_msg_buffer\n";
    rpc->resize_msg_buffer(&req, sizeof(CommonMsg));
    cout << "xihu read begin to enqueue_request\n";
    rpc->enqueue_request(session, kReqType, &req, &resp, cont_func, (void *)k);
  }
  cout << "read end to loop\n";
  // 数据请求仍实现为阻塞式
  while (ms.nxt < ms.tot + ms.pip) rpc->run_event_loop_once();
  return 0;
}