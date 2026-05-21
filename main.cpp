#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <thread>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

void initCpuStats() {
    ifstream file("/proc/stat");
    string line, cpu;
    if (getline(file, line)) {
        istringstream ss(line);
        ss >> cpu >> lastTotalUser >> lastTotalUserLow >> lastTotalSys >> lastTotalIdle;
    }
}

double getCpuUsage() {
    ifstream file("/proc/stat");
    string line, cpu;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;
    if (getline(file, line)) {
        istringstream ss(line);
        ss >> cpu >> totalUser >> totalUserLow >> totalSys >> totalIdle;
        
        if (totalUser < lastTotalUser || totalSys < lastTotalSys || totalIdle < lastTotalIdle) return 0.0;
        
        unsigned long long totalDifference = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) + (totalSys - lastTotalSys);
        total = totalDifference + (totalIdle - lastTotalIdle);
        
        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;
        
        return total == 0 ? 0.0 : (totalDifference * 100.0) / total;
    }
    return 0.0;
}

json getSystemStats() {
    json stats;

    struct utsname buffer;
    if (uname(&buffer) == 0) {
        stats["hostname"] = buffer.nodename;
        stats["os"] = buffer.sysname;
        stats["kernel"] = buffer.release;
    }

    ifstream uptimeFile("/proc/uptime");
    double uptimeStr;
    if (uptimeFile >> uptimeStr) {
        stats["uptime_seconds"] = uptimeStr;
    }

    ifstream memFile("/proc/meminfo");
    string key, unit;
    long long value, memTotal = 0, memAvailable = 0;
    while (memFile >> key >> value >> unit) {
        if (key == "MemTotal:") memTotal = value;
        if (key == "MemAvailable:") memAvailable = value;
    }
    stats["ram_total_mb"] = memTotal / 1024;
    stats["ram_free_mb"] = memAvailable / 1024;
    stats["ram_usage_percent"] = memTotal == 0 ? 0 : 100.0 * (memTotal - memAvailable) / memTotal;

    ifstream cpuFile("/proc/cpuinfo");
    string line;
    int cores = 0;
    string modelName = "Unknown";
    while (getline(cpuFile, line)) {
        if (line.find("processor") == 0) cores++;
        if (line.find("model name") == 0) {
            modelName = line.substr(line.find(":") + 2);
        }
    }
    stats["cpu_cores"] = cores;
    stats["cpu_model"] = modelName;

    stats["cpu_usage_percent"] = getCpuUsage();

    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        unsigned long long total = stat.f_blocks * stat.f_frsize;
        unsigned long long available = stat.f_bavail * stat.f_frsize;
        stats["disk_total_gb"] = total / (1024.0 * 1024.0 * 1024.0);
        stats["disk_free_gb"] = available / (1024.0 * 1024.0 * 1024.0);
        stats["disk_usage_percent"] = total == 0 ? 0 : 100.0 * (total - available) / total;
    }

    ifstream netFile("/proc/net/dev");
    string netLine;
    long long rxBytes = 0, txBytes = 0;
    while (getline(netFile, netLine)) {
        if (netLine.find("eth0") != string::npos || netLine.find("en") != string::npos) {
            istringstream ss(netLine.substr(netLine.find(":") + 1));
            long long temp;
            ss >> rxBytes >> temp >> temp >> temp >> temp >> temp >> temp >> temp >> txBytes;
            break;
        }
    }
    stats["network_rx_mb"] = rxBytes / (1024.0 * 1024.0);
    stats["network_tx_mb"] = txBytes / (1024.0 * 1024.0);

    ifstream loadFile("/proc/loadavg");
    string l1, l2, l3, procs;
    if (loadFile >> l1 >> l2 >> l3 >> procs) {
        stats["active_tasks"] = procs;
    }

    return stats;
}

int main() {
    initCpuStats();
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        ifstream file("index.html");
        stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    svr.Get("/api/stats", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(getSystemStats().dump(), "application/json");
    });

    cout << "Server starting on http://0.0.0.0:8080" << endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}
