#include <iostream>
#include <memory>
#include "trpc/common/trpc_app.h"
#include "services.h"

class RpcServer final : public trpc::TrpcApp {
 public:
  int Initialize() override {
    if (RegisterService("trpc.homework.rpc.Greeter", std::make_shared<GreeterService>()) !=
        trpc::TrpcServer::RegisterRetCode::kOk) return -1;
    if (RegisterService("trpc.homework.rpc.Calc", std::make_shared<CalcService>()) !=
        trpc::TrpcServer::RegisterRetCode::kOk) return -1;
    std::cout << "Registered Greeter and Calc" << std::endl;
    return 0;
  }
  void Destroy() override {}
};
int main(int argc, char** argv) {
  RpcServer server;
  int result = server.Main(argc, argv);
  if (result != 0) return result;
  server.Wait();
  return 0;
}
