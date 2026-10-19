#include "services.h"
#include <cstdint>
#include <iostream>

trpc::Status GreeterService::SayHello(trpc::ServerContextPtr,
    const homework::HelloRequest* req, homework::HelloReply* rsp) {
  rsp->set_msg("Hello, " + req->msg());
  std::cout << "SayHello: " << rsp->msg() << std::endl;
  return trpc::kSuccStatus;
}
trpc::Status CalcService::Add(trpc::ServerContextPtr,
    const homework::CalcRequest* req, homework::CalcReply* rsp) {
  rsp->set_result(static_cast<std::int64_t>(req->a()) + req->b());
  std::cout << "Add(" << req->a() << ", " << req->b() << ") = " << rsp->result() << std::endl;
  return trpc::kSuccStatus;
}
trpc::Status CalcService::Mul(trpc::ServerContextPtr,
    const homework::CalcRequest* req, homework::CalcReply* rsp) {
  rsp->set_result(static_cast<std::int64_t>(req->a()) * req->b());
  std::cout << "Mul(" << req->a() << ", " << req->b() << ") = " << rsp->result() << std::endl;
  return trpc::kSuccStatus;
}
