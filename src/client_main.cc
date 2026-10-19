#include <cstdint>
#include <iostream>
#include "gflags/gflags.h"
#include "trpc/client/make_client_context.h"
#include "trpc/client/trpc_client.h"
#include "trpc/common/runtime_manager.h"
#include "proto/helloworld.trpc.pb.h"
#include "proto/calc.trpc.pb.h"

DEFINE_string(client_config, "conf/client.yaml", "Client YAML path");
DEFINE_bool(healthcheck, false, "Check both services without full test suite");

int Run() {
  auto greeter = trpc::GetTrpcClient()->GetProxy<homework::GreeterServiceProxy>("trpc.homework.rpc.Greeter");
  auto calc = trpc::GetTrpcClient()->GetProxy<homework::CalcServiceProxy>("trpc.homework.rpc.Calc");
  homework::HelloRequest hello;
  hello.set_msg("tRPC");
  homework::HelloReply reply;
  auto status = greeter->SayHello(trpc::MakeClientContext(greeter), hello, &reply);
  if (!status.OK()) {
    std::cerr << "RPC_ERROR SayHello: " << status.ToString() << std::endl;
    return 2;
  }
  if (reply.msg() != "Hello, tRPC") {
    std::cerr << "FAIL SayHello: unexpected response " << reply.msg() << std::endl;
    return 1;
  }
  std::cout << "PASS SayHello => " << reply.msg() << std::endl;
  struct Case { bool multiply; std::int32_t a, b; std::int64_t expected; };
  const Case cases[] = {
    {false, 3, 5, 8}, {true, 3, 5, 15},
    {false, -3, 5, 2}, {true, 0, 5, 0}, {true, -3, 5, -15},
    {false, 2147483647, 2147483647, 4294967294LL},
    {false, INT32_MIN, INT32_MIN, -4294967296LL},
    {true, INT32_MIN, INT32_MIN, 4611686018427387904LL},
    {true, INT32_MIN, INT32_MAX, -4611686016279904256LL}
  };
  int count = 1;
  for (const auto& test : cases) {
    homework::CalcRequest request;
    request.set_a(test.a);
    request.set_b(test.b);
    homework::CalcReply response;
    auto ctx = trpc::MakeClientContext(calc);
    status = test.multiply ? calc->Mul(ctx, request, &response) : calc->Add(ctx, request, &response);
    const char* method = test.multiply ? "Mul" : "Add";
    if (!status.OK()) {
      std::cerr << "RPC_ERROR " << method << ": " << status.ToString() << std::endl;
      return 2;
    }
    if (response.result() != test.expected) {
      std::cerr << "FAIL " << method << ": expected " << test.expected << ", got " << response.result() << std::endl;
      return 1;
    }
    std::cout << "PASS " << method << "(" << test.a << ", " << test.b << ") => " << response.result() << std::endl;
    ++count;
    if (FLAGS_healthcheck) break;
  }
  std::cout << "ALL " << count << " RPC CHECKS PASSED" << std::endl;
  return 0;
}
int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (trpc::TrpcConfig::GetInstance()->Init(FLAGS_client_config) != 0) {
    std::cerr << "CONFIG_ERROR: " << FLAGS_client_config << std::endl;
    return 3;
  }
  return trpc::RunInTrpcRuntime([] { return Run(); });
}
