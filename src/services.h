#pragma once
#include "proto/helloworld.trpc.pb.h"
#include "proto/calc.trpc.pb.h"

class GreeterService final : public homework::Greeter {
 public:
  trpc::Status SayHello(trpc::ServerContextPtr, const homework::HelloRequest*,
                       homework::HelloReply*) override;
};
class CalcService final : public homework::Calc {
 public:
  trpc::Status Add(trpc::ServerContextPtr, const homework::CalcRequest*,
                  homework::CalcReply*) override;
  trpc::Status Mul(trpc::ServerContextPtr, const homework::CalcRequest*,
                  homework::CalcReply*) override;
};
