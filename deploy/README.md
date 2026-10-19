# 阿里云 ECS 部署与管理

已于 2026-10-19 13:21（北京时间）完成部署和验收。

## 已部署的环境

- ECS：8.148.65.190，Ubuntu 26.04.1 LTS，x86_64，2 核／2GB。
- 远程工程目录：`/opt/trpc-homework`。
- Docker Engine 29.1.3，Compose 2.40.3，来自 Ubuntu 的软件仓库（ECS 使用阿里云 Ubuntu 镜像源）。
- Docker 官方源连接被重置，已将本次添加的源保存为 `/etc/apt/sources.list.d/docker.sources.disabled`，改用 Ubuntu 的 `docker.io` 和 `docker-compose-v2` 软件包。
- 上传的是本机已经编译验证的 `trpc-homework:1.0` 镜像，未在 2GB 内存服务器重新编译。
- 镜像归档 `trpc-homework-image.tar.gz` 为 42357976 字节；SHA256 记录在同目录 `image.sha256`，服务器校验通过。
- 服务端常驻运行，客户端按需启动并自动删除。Docker 已启用开机启动，服务端设置 `restart: unless-stopped`。
- 无主机端口映射，RPC 仅通过 Docker 内部网络通信；无需浏览器访问公网 IP，也没有部署网页。

## 从 Windows 连接

在本机 PowerShell 执行：

```powershell
ssh -i "$env:USERPROFILE/.ssh/trpc_ecs_8_148_65_190" root@8.148.65.190
```

专用私钥存储于当前 Windows 用户的 `.ssh` 目录，不在工程或压缩包中。公钥已追加至服务器 `/root/.ssh/authorized_keys`，原有授权条目保留。未修改 root 密码或 SSH 密码登录设置。

## 登录服务器后的常用命令

```bash
cd /opt/trpc-homework

# 查看健康状态
docker compose ps

# 单独运行 10 项客户端验证
docker compose run --rm --no-deps client

# 查看服务端日志
docker compose logs --tail 50 server

# 完整验收：会短暂停服并测试失败场景，然后恢复
bash scripts/verify.sh

# 手动停止服务
docker compose stop server

# 重新启动并等待健康
docker compose up -d --wait --wait-timeout 120 server
```

主动 stop 后，unless-stopped 不会自行恢复该服务；使用最后一条命令重新启动即可。

## 真实验收结果

- 10 项真实 RPC 检查全部通过，包含 HelloWorld、普通加法／乘法、负数、零和 int32 边界。
- 停服后约 2 秒返回 `RPC_ERROR`，客户端退出码为 2。原因是停止容器后服务名无法解析，属于本次预期不可用场景，不代表已模拟所有网络超时情况。
- 验收完成后服务端恢复 `healthy`。
- 一次资源快照：服务端内存约 111.2 MiB、CPU 约 1.80%，仅代表该次采样，不是性能压测结果。
- 云端日志：`/opt/trpc-homework/evidence/runs/20261019-132044/`。
- 下载到本机的日志：`../evidence/cloud/evidence/runs/20261019-132044/`。
- 云端最终状态快照：`../evidence/cloud/evidence/cloud-status.log`。
- 安装日志：`../evidence/cloud/trpc-docker-ubuntu-install.log`。

## 重新导入与启动

同目录的 Compose 配置专用于运行已上传镜像，不执行构建或在线拉取：

```bash
cd /opt/trpc-homework
sha256sum -c image.sha256
docker load -i trpc-homework-image.tar.gz
docker compose up -d --wait --wait-timeout 120 server
```

也可以在云端从源码构建，或使用镜像仓库分发。本次选择压缩镜像上传，是因为本机已有通过验证的镜像，能够节省云端编译资源，并避开云服务器访问 Docker Hub 的网络不确定性。
