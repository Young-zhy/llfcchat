#pragma once
#include "Singleton.h"
#include "const.h"
#include "message.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPoll {
public:
  RPConPoll(size_t poolsize, std::string host, std::string port)
      : poolSize_(poolsize), host_(host), port_(port), b_stop_(false) {
    for (size_t i = 0; i < poolSize_; ++i) {
      std::shared_ptr<Channel> channel = grpc::CreateChannel(
          host + ":" + port, grpc::InsecureChannelCredentials());
      connections_.push(VarifyService::NewStub(channel));
    }
  }
  ~RPConPoll() {
    std::lock_guard<std::mutex> lock(mutex_);
    Close();
    while (!connections_.empty()) {
      connections_.pop();
    }
  }

  void Close() {
    b_stop_ = true;
    cond_.notify_all();
  }

  std::unique_ptr<VarifyService::Stub> getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() {
      if (b_stop_)
        return true;                // 退出等待
      return !connections_.empty(); // 连接池不为空时继续
    });

    if (b_stop_) {
      return nullptr; // 连接池已关闭，返回空指针
    }

    auto context = std::move(connections_.front());
    connections_.pop();
    return context;
  }

  void returnConnection(std::unique_ptr<VarifyService::Stub> context) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_) {
      return; // 连接池已关闭，不接受返回的连接
    }

    connections_.push(std::move(context));
    cond_.notify_one();
  }

private:
  std::atomic<bool> b_stop_;
  size_t poolSize_;
  std::string host_;
  std::string port_;
  std::queue<std::unique_ptr<VarifyService::Stub>> connections_{};
  std::condition_variable cond_;
  std::mutex mutex_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
  friend class Singleton<VerifyGrpcClient>;

public:
  GetVarifyRsp GetVarifyCode(std::string email) {
    ClientContext context;
    GetVarifyRsp reply;
    GetVarifyReq request;
    request.set_email(email);
    auto stub = pool_->getConnection();
    Status status = stub->GetVarifyCode(&context, request, &reply);

    if (status.ok()) {
      pool_->returnConnection(std::move(stub));
      return reply;
    } else {
      pool_->returnConnection(std::move(stub));
      reply.set_error(ErrorCodes::RPCFailed);
      return reply;
    }
  }

private:
  VerifyGrpcClient();

  std::unique_ptr<RPConPoll> pool_;
};
