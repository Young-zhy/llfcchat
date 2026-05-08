#include "CServer.h"
#include "AsioIOServicePool.h"
#include "HttpConnection.h"
#include <iostream>

CServer::CServer(boost::asio::io_context &ioc, unsigned short &port)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {}

void CServer::Start() {
  auto self = shared_from_this();
  auto &io_context = AsioIOServicePool::GetInstance()->GetIOService();
  std::shared_ptr<HttpConnection> new_conn =
      std::make_shared<HttpConnection>(io_context);
  _acceptor.async_accept(
      new_conn->GetSocket(), [self, new_conn](beast::error_code ec) {
        try {
          // 出错则放弃这个连接，继续监听新链接
          if (ec) {
            self->Start();
            return;
          }

          // 处理新链接，创建HpptConnection类管理新连接
          new_conn->Start();

          // 继续监听
          self->Start();
        } catch (std::exception &exp) {
          std::cout << "exception is " << exp.what() << std::endl;
          self->Start();
        }
      });
}
