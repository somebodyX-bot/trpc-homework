# 本机验收记录

验收日期：2026-10-18，Asia/Shanghai。

## 环境与结果

- 主机：Windows + Docker Desktop，Linux/amd64 容器。
- Docker Engine：29.8.2；Compose：v5.6.0。
- 编译：Ubuntu 22.04、GCC 11.4.0、CMake 3.22.1、C++17。
- tRPC-Cpp v1.2.0、protobuf v3.15.8；16 份源码归档通过 SHA256 校验。
- tRPC 框架、rpc_server、rpc_client 均在容器内从源码编译通过。
- 最终镜像：`trpc-homework:1.0`，linux/amd64，169577780 字节。
- 最终镜像 ID：`sha256:ca632eaab149a7ea24c29caf59316f572ef102062317926017a988a6462611d8`。
- 最终验收日志目录：`runs/20261018-210231/`。
- 服务端最终状态：`healthy`；没有发布主机端口。

## 真实调用

| 验证 | 预期与实际结果 |
|---|---|
| SayHello("tRPC") | Hello, tRPC |
| Add(3, 5) | 8 |
| Mul(3, 5) | 15 |
| Add(-3, 5) | 2 |
| Mul(0, 5) | 0 |
| Mul(-3, 5) | -15 |
| Add(INT32_MAX, INT32_MAX) | 4294967294 |
| Add(INT32_MIN, INT32_MIN) | -4294967296 |
| Mul(INT32_MIN, INT32_MIN) | 4611686018427387904 |
| Mul(INT32_MIN, INT32_MAX) | -4611686016279904256 |
| 停止服务后调用 | RPC_ERROR，退出码 2，约 2.11 秒 |
| 恢复服务 | 健康检查成功 |

停服测试中，Docker 服务名失去可用地址，tRPC 的 domain 选择器报错；这是预期失败，日志中保留真实错误。它验证了不可用服务不会被误报为调用成功，但不代表模拟了所有丢包、网络黑洞等超时情形。RPC 配置超时为 2000 ms。

## 构建证据及复现说明

`build.log` 保存框架和业务代码的完整构建过程，`build-final.log` 保存最终镜像构建结果。此次使用刚从 Ubuntu 22.04 安装工具得到的本地缓存镜像，避免重复下载系统包：

```text
docker build --progress=plain --build-arg BUILD_BASE=trpc-homework-tools:local -t trpc-homework:1.0 .
```

缓存镜像只预装编译工具，没有预置 tRPC 或业务二进制。Dockerfile 默认 `BUILD_BASE=ubuntu:22.04`，包含安装工具的步骤，迁移到其他电脑不需要这个本地缓存镜像。默认基础镜像的全新下载构建路径尚未独立重跑；本次实际验证的是上述缓存工具基础之上的完整源码编译和最终镜像。

Windows 验收脚本已实际执行；Linux 验收脚本已通过 `bash -n` 语法检查，尚未在云服务器执行。云部署、K8s、mini-DHT 不属于本次完成范围。容器内部框架日志时间为 UTC，比本记录的北京时间早 8 小时（即 13:02 UTC 对应 21:02 北京时间）。
