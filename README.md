# tRPC-Cpp 课程作业：HelloWorld + Calc

2026-10-19 已完成本地容器构建、阿里云 ECS 部署及云端验收，详见 [云端部署与管理](deploy/README.md)。

本工程对应课件的 RPC 编程作业：使用 C++、Protocol Buffers 和 tRPC-Cpp，完成真实客户端／服务端调用，并提供 CMake 构建、Docker Compose 部署及阿里云 ECS 运行证据。不包含 Kubernetes 或 mini-DHT。

## 功能与结构

一个服务端进程注册两个服务，客户端作为另一个容器运行，通过 Compose 内部网络访问。

| 服务 | 方法 | 输入／输出 | 端口 |
|---|---|---|---|
| Greeter | SayHello | `msg` → `Hello, ` + `msg` | 8000 |
| Calc | Add | 两个 int32 → int64 和 | 8001 |
| Calc | Mul | 两个 int32 → int64 积 | 8001 |

计算前将操作数提升到 int64，保证两个 int32 的任意加法、乘法结果均不会发生有符号整数溢出。服务端监听 `0.0.0.0`；客户端使用 `server:8000/8001` 和 tRPC 的 domain 选择器解析容器服务名。每次 RPC 超时为 2000 ms。默认没有向宿主机或公网发布端口。

```text
proto/       接口定义，构建时自动生成 protobuf 与 tRPC 代码
src/         服务实现、服务端入口、自动验收客户端
conf/        服务端、跨容器客户端及本机健康检查 YAML
scripts/     依赖校验解包、Windows/Linux 验收脚本
vendor/      官方源码归档及版本、URL、SHA256 清单
evidence/    真实构建日志与各次验收日志
Dockerfile   多阶段编译与运行镜像
compose.yaml 服务编排和真实 RPC 健康检查
```

## 环境与构建

需要 Docker Engine / Docker Desktop 的 Linux 容器模式，以及支持 `up --wait` 的 Docker Compose。Windows 不需要另外安装 Ubuntu、CMake 或 C++ 编译器。

框架固定为 **tRPC-Cpp v1.2.0**，protobuf **v3.15.8**，C++17，Ubuntu 22.04。依赖版本沿用该框架的 CMake 配置；上游未固定的 picohttpparser 已固定至具体提交。所有 16 份源码归档及 SHA256 在 `vendor/sources.json` 中记录。镜像构建先校验归档，再让 CMake 使用本地依赖，避免构建过程中克隆 GitHub。首次拉取 Ubuntu 镜像、安装系统编译工具仍需要网络。

在本目录执行：

```powershell
docker compose --progress plain build
docker compose up -d --wait --wait-timeout 120 server
docker compose run --rm --no-deps client
```

正常调用输出 `Hello, tRPC`、`Add(3, 5) => 8`、`Mul(3, 5) => 15`，全部通过时输出 `ALL 10 RPC CHECKS PASSED`，退出码为 0。

框架首次编译需要数分钟，默认 4 个编译任务。内存不足时，在 PowerShell 中先设置 `$env:BUILD_JOBS='2'`；Linux 中使用 `BUILD_JOBS=2 docker compose build`。

Dockerfile 内的核心构建步骤是：

```bash
python3 /tmp/prepare_vendor.py /tmp/vendor /opt/trpc
cmake -S /opt/trpc -B /opt/trpc/build -C /opt/trpc/offline.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build /opt/trpc/build --target trpc -j4
cmake -S /src -B /src/build -DTRPC_ROOT_PATH=/opt/trpc -DCMAKE_BUILD_TYPE=Release
cmake --build /src/build -j4
```

运行镜像仅包含程序、配置、必要动态库，使用普通用户运行。构建工具和框架源码保留在 builder 阶段，不进入最终运行镜像。

## 一键验收及证据

Windows PowerShell：

```powershell
./scripts/verify.ps1
# 包含镜像构建：
./scripts/verify.ps1 -Build
```

Linux 从源码构建并验收：

```bash
bash scripts/verify.sh --build
```

已部署的 ECS 使用 `/opt/trpc-homework` 中的运行配置和已导入镜像，直接执行 `bash scripts/verify.sh`，不加 `--build`。

脚本验证 10 次真实 RPC：HelloWorld、普通加法和乘法、负数、零、int32 上下界加法、两个最小整数相乘、最小整数乘最大整数。随后停止服务端，确认客户端以 RPC 错误退出码 2 失败；最后恢复服务端。

客户端退出码：0 成功、1 返回结果不符、2 RPC 通信失败、3 配置初始化失败。健康检查同样调用 Greeter 和 Calc，确保业务接口可用，而不仅检查端口。

每次验收写入 `evidence/runs/时间戳/`，包括 `client.log`、`server.log`、`unavailable.log`、`negative-result.log`、`containers.log` 和 `result.log`。建议提交时保留这些日志，并自行截取终端中的成功输出，理解每个验证项后再提交。

常用管理命令：

```powershell
docker compose ps
docker compose logs --tail 50 server
docker compose stop server
docker compose start server
docker compose down
```

## 实现选择与替代方法

选择 CMake 是为了对齐课程；选择 HelloWorld＋计算器是为了完整覆盖接口定义、代码生成、注册、调用和边界验证，同时不引入数据库等额外依赖。选择 Docker Compose 是为了复用现有 Docker Desktop 环境，让客户端、服务端使用相同程序和依赖，后续可将整个目录上传到 Linux 云服务器构建。

其他可行方式包括独立 WSL Ubuntu 开发、Bazel 构建或在云端源码构建。本工程采用本地容器内 CMake 构建，然后将同一镜像导入云服务器运行，减少主机环境差异及 2GB 云服务器的编译内存压力。

## 来源与 AI 使用说明

- 课程要求来自上级目录 `L4.pdf`，AI 使用说明要求来自 `L1.pdf`。
- 官方框架及示例：https://github.com/trpc-group/trpc-cpp/tree/v1.2.0/examples/helloworld
- 使用的第三方源码归档保留原始许可证；具体来源见 `vendor/sources.json`。独立客户端和服务端生命周期参考官方示例，业务接口、边界验收、容器配置和说明由本工程实现。
- AI 协助：读取此前讨论、搭建工程、实现 C++/proto/CMake/配置/脚本、排查构建问题并执行验证。学生本人仍需阅读代码、理解 RPC 流程、运行复核，并如实填写自己亲自修改的部分；这里不代填未实际发生的个人工作。
