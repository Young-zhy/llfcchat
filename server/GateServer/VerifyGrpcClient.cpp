#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

VerifyGrpcClient::VerifyGrpcClient() {
  auto &gCfgMgr = ConfigMgr::Inst();
  std::string host = gCfgMgr["VarifyServer"]["Host"];
  std::string port = gCfgMgr["VarifyServer"]["Port"];
  pool_.reset(new RPConPoll(10, host, port));
}
