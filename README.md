# 🚀 Ultra-Lightweight C++ System Dashboard

[![License](https://img.shields.io/github/license/tiwut/System-Info-Dashboard?style=flat-square&color=blue)](https://github.com/tiwut/System-Info-Dashboard/blob/main/LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/tiwut/System-Info-Dashboard?style=flat-square&color=gold)](https://github.com/tiwut/System-Info-Dashboard/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/tiwut/System-Info-Dashboard?style=flat-square&color=orange)](https://github.com/tiwut/System-Info-Dashboard/network/members)
[![GitHub issues](https://img.shields.io/github/issues/tiwut/System-Info-Dashboard?style=flat-square)](https://github.com/tiwut/System-Info-Dashboard/issues)
[![GitHub last commit](https://img.shields.io/github/last-commit/tiwut/System-Info-Dashboard?style=flat-square)](https://github.com/tiwut/System-Info-Dashboard/commits/main)


A lightning-fast, self-contained system information dashboard. The backend is written entirely in optimized C++ to directly parse Linux kernel metrics from the `/proc` filesystem, serving them to a beautiful, responsive Tailwind CSS web UI.

Because it is compiled directly to machine code in a multi-stage Docker build, this image is exceptionally small and consumes practically zero CPU overhead while monitoring your server.

## ✨ Features (14 Real-Time Metrics)
The dashboard auto-refreshes every 1.5 seconds to display:
- **System:** Hostname, OS Name, Kernel Version, Uptime, Online Status
- **CPU:** Real-time Usage (%), Core Count, Model Name, Active Tasks (Load)
- **Memory:** Total RAM, Free RAM, RAM Usage (%)
- **Storage:** Disk Total Space, Disk Free Space, Disk Usage (%)
- **Network:** Rx (Received) and Tx (Transmitted) traffic

---

## 🐳 Quick Start

### Option 1: Docker CLI
To run the container in the background using standard port forwarding, execute:

```bash
docker run -d \
  --name cpp-sysinfo \
  -p 8080:8080 \
  --pid=host \
  tiwutdev/system-info-dashboard:latest
```

### Option 2: Docker Compose (Recommended)
Create a `docker-compose.yml` file:

```yaml
version: '3.8'

services:
  sysinfo-app:
    image: tiwutdev/system-info-dashboard:latest
    container_name: cpp-sysinfo
    restart: unless-stopped
    ports:
      - "8080:8080"
    pid: "host"
```
Then start it up:
```bash
docker compose up -d
```

**Access the dashboard:** Open your browser and navigate to `http://<your-server-ip>:8080`.

---

## ⚠️ Important Configuration Notes

To get the most accurate hardware metrics, you must understand how Docker isolates environments:

1. **`pid: "host"` (Required)**
   By default, Docker hides host processes from containers. Passing `--pid=host` allows the C++ backend to read the actual number of active tasks on the host machine from `/proc/loadavg`. Without it, your active task count will constantly show `0` or `1`.

2. **Port Forwarding vs. Host Networking**
   Because this configuration uses Port Forwarding (`-p 8080:8080`), the Network Rx/Tx metrics will show the traffic of the **container's isolated virtual network**, not the host machine's physical network card.
   * *Want accurate host network traffic?* Remove `ports: - "8080:8080"` and replace it with `network_mode: "host"` in your compose file.

3. **Disk Metrics**
   The disk space metrics read the partition where Docker's overlay network is stored (usually `/var/lib/docker`). In 99% of setups, this accurately reflects the main root drive of the host machine.

---

## 🛠️ Built With
* **Backend:** C++11 (GCC)
* **Libraries:** [cpp-httplib](https://github.com/yhirose/cpp-httplib), [nlohmann/json](https://github.com/nlohmann/json)
* **Frontend:** HTML5, Vanilla JavaScript, Tailwind CSS
* **Base Image:** Debian Bookworm Slim
