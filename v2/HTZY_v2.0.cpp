// HTZY_v2.0 - Main Entry
// Starts C++ backend (port 8080) + auto-launches Vite dev server (port 5173)
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include "common/config.h"
#include "common/logger.h"
#include "db/database.h"
#include "db/schema_migrator.h"
#include "server/http_server.h"

// Auto-launch Vite dev server in background
static void startViteDevServer(const std::string& frontend_dir) {
    LOG_INFO("Boot", "Starting Vite dev server in " + frontend_dir + "...");
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    std::string cmd = "cmd /c cd /d \"" + frontend_dir + "\" && node node_modules/vite/bin/vite.js --port 5173 --host";
    if (CreateProcessA(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, FALSE,
                       CREATE_NO_WINDOW, NULL, frontend_dir.c_str(), &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        LOG_INFO("Boot", "Vite dev server launched (PID: " + std::to_string(pi.dwProcessId) + ")");
    } else {
        LOG_WARN("Boot", "Failed to launch Vite");
    }
#else
    std::string cmd = "cd " + frontend_dir + " && nohup node node_modules/vite/bin/vite.js --port 5173 --host > /dev/null 2>&1 &";
    system(cmd.c_str());
    LOG_INFO("Boot", "Vite dev server launched on Linux");
#endif
    LOG_INFO("Boot", "Frontend: http://localhost:5173");
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    std::cout << "HTZY v2.0 - AI Interview Platform" << std::endl;
    std::cout << "==================================" << std::endl;

    // 0. Ensure CWD = exe directory (so config/uploads/etc are found correctly)
#ifdef _WIN32
    char exe_buf[MAX_PATH];
    GetModuleFileNameA(NULL, exe_buf, MAX_PATH);
    std::string exe_path(exe_buf);
    size_t sep = exe_path.find_last_of("\\/");
    if (sep != std::string::npos) {
        std::string exe_dir = exe_path.substr(0, sep);
        SetCurrentDirectoryA(exe_dir.c_str());
        LOG_INFO("Boot", "Working dir: " + exe_dir);
    }
#else
    // Linux: read /proc/self/exe
    char exe_buf[4096];
    ssize_t len = readlink("/proc/self/exe", exe_buf, sizeof(exe_buf) - 1);
    if (len > 0) {
        exe_buf[len] = '\0';
        std::string exe_path(exe_buf);
        size_t sep = exe_path.find_last_of('/');
        if (sep != std::string::npos) {
            chdir(exe_path.substr(0, sep).c_str());
        }
    }
#endif

    // 1. Load config
    std::string config_path = "config/app.conf";
    if (argc > 1) config_path = argv[1];

    Config& config = Config::instance();
    if (!config.load(config_path)) {
        // Try creating config directory and copying template
        std::string template_path = "config/app.conf.example";
        std::ifstream src(template_path, std::ios::binary);
        if (src.is_open()) {
#ifdef _WIN32
            CreateDirectoryA("config", NULL);
#else
            mkdir("config", 0755);
#endif
            std::ofstream dst(config_path, std::ios::binary);
            dst << src.rdbuf();
            dst.close();
            src.close();
            std::cout << "[Boot] Created app.conf from template. Restarting..." << std::endl;
            // Reload instead of exiting
            if (!config.load(config_path)) {
                std::cerr << "[Boot] Failed to load config even after creation." << std::endl;
                return 1;
            }
        } else {
            std::cerr << "[Boot] No config found and no template to copy." << std::endl;
            return 1;
        }
    }

    const AppConfig& cfg = config.get();

    // 2. Init logger
    Logger::instance();
    if (!cfg.log_file.empty()) {
        Logger::instance().setLogFile(cfg.log_file);
    }
    LOG_INFO("Boot", "Config loaded. Server: " + cfg.server.host + ":" + std::to_string(cfg.server.port));

    // 3. Init database (idempotent)
    if (cfg.database.auto_init) {
        LOG_INFO("Boot", "Initializing database...");
        if (!SchemaMigrator::migrate(cfg)) {
            LOG_ERROR("Boot", "DB init failed.");
            return 1;
        }
    }

    // 4. Connect business DB
    Database& db = Database::instance();
    if (!db.connect(cfg.database.host, cfg.database.port,
                    cfg.database.user, cfg.database.password,
                    cfg.database.dbname)) {
        LOG_ERROR("Boot", "Business DB connection failed");
        return 1;
    }

    // 5. Auto-launch Vite dev server (frontend hot-reload)
    std::string fe_dir = "frontend";
    std::string fe_check_path = fe_dir + "/package.json";
    std::ifstream fe_check(fe_check_path);
    if (fe_check.is_open()) {
        fe_check.close();
#ifdef _WIN32
        char full_path[MAX_PATH];
        if (GetFullPathNameA(fe_dir.c_str(), MAX_PATH, full_path, NULL)) {
            startViteDevServer(std::string(full_path));
        } else {
            startViteDevServer(fe_dir);
        }
#else
        startViteDevServer(fe_dir);
#endif
    } else {
        LOG_INFO("Boot", "No frontend/package.json, using static dist only");
    }

    // 6. Register HTTP routes and start server
    HttpServer& http = HttpServer::instance();
    http.setStaticDir(cfg.server.static_dir);
    http.registerRoutes(cfg.server.jwt_secret);

    LOG_INFO("Boot", "HTZY v2.0 started!");
    LOG_INFO("Boot", "Backend API:   http://localhost:" + std::to_string(cfg.server.port));
    LOG_INFO("Boot", "Frontend Dev:  http://localhost:5173 (auto-proxied)");
    LOG_INFO("Boot", "Press Ctrl+C to stop");

    if (!http.start(cfg.server.host, cfg.server.port)) {
        LOG_ERROR("Boot", "Server failed to start");
        return 1;
    }

    db.disconnect();
    LOG_INFO("Boot", "Service stopped");
    return 0;
}
