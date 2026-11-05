// Mailer - SMTP email sender
// Windows: via curl + CreateProcess
// Linux: via fork+exec curl (no temp files, no shell)
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "logger.h"
#include "config.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#endif

class Mailer {
public:
    static Mailer& instance() { static Mailer i; return i; }

    bool send(const std::string& to, const std::string& subject, const std::string& body) {
        const auto& cfg = Config::instance().get().smtp;
        if (cfg.host.empty() || cfg.user.empty() || cfg.password.empty()) {
            LOG_ERROR("Mailer", "SMTP not configured");
            return false;
        }

        std::string encoded_subject = base64Encode(subject);
        std::string encoded_body = base64Encode(body);
        std::string encoded_name = base64Encode(cfg.from_name);

        std::string mime;
        mime += "From: =?UTF-8?B?" + encoded_name + "?= <" + cfg.user + ">\r\n";
        mime += "To: " + to + "\r\n";
        mime += "Subject: =?UTF-8?B?" + encoded_subject + "?=\r\n";
        mime += "MIME-Version: 1.0\r\n";
        mime += "Content-Type: text/plain; charset=UTF-8\r\n";
        mime += "Content-Transfer-Encoding: base64\r\n";
        mime += "\r\n";
        mime += encoded_body;

        // Write to temp file
        std::string tmp_file = "/tmp/htzy_mail.eml";
        {
            std::ofstream ofs(tmp_file, std::ios::binary);
            if (!ofs.is_open()) {
                LOG_ERROR("Mailer", "Cannot write temp file");
                return false;
            }
            ofs << mime;
        }

        LOG_INFO("Mailer", "Sending email to " + to + "...");

#ifdef _WIN32
        // Windows: use CreateProcess with cmd /c curl
        std::string curl_cmd = "curl.exe -s --ssl-reqd "
            "--url smtps://" + cfg.host + ":" + std::to_string(cfg.port) + " "
            "--user " + cfg.user + ":" + cfg.password + " "
            "--mail-from " + cfg.user + " "
            "--mail-rcpt " + to + " "
            "-T " + tmp_file + " "
            "--max-time 30";
        std::string cmd_line = "cmd /c " + curl_cmd + " 2>nul";

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};
        int ret = -1;
        if (CreateProcessA(NULL, const_cast<char*>(cmd_line.c_str()), NULL, NULL, FALSE,
                           CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 30000);
            DWORD exit_code = 0;
            GetExitCodeProcess(pi.hProcess, &exit_code);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            ret = (int)exit_code;
        }
#else
        // Linux: fork+exec curl directly
        std::vector<std::string> args;
        args.push_back("curl");
        args.push_back("-s");
        args.push_back("--ssl-reqd");
        args.push_back("--url");
        args.push_back("smtps://" + cfg.host + ":" + std::to_string(cfg.port));
        args.push_back("--user");
        args.push_back(cfg.user + ":" + cfg.password);
        args.push_back("--mail-from");
        args.push_back(cfg.user);
        args.push_back("--mail-rcpt");
        args.push_back(to);
        args.push_back("-T");
        args.push_back(tmp_file);
        args.push_back("--max-time");
        args.push_back("30");

        int ret = forkExec(args);
#endif

        std::remove(tmp_file.c_str());

        if (ret == 0) {
            LOG_INFO("Mailer", "Email sent successfully");
            return true;
        } else {
            LOG_ERROR("Mailer", "Email send failed with code " + std::to_string(ret));
            return false;
        }
    }

    bool sendVerifyCode(const std::string& to, const std::string& code) {
        std::string subject = "HTZY Verification Code";
        std::string body = "Hello,\n\nYour verification code is: " + code + "\n\nThis code expires in 10 minutes.\n\n- HTZY Team";
        return send(to, subject, body);
    }

    bool sendDeliveryNotice(const std::string& to, const std::string& name,
                           const std::string& company, const std::string& position) {
        std::string subject = "HTZY Resume Delivery Notice";
        std::string body = "Hello " + name + ",\n\nYour resume has been delivered:\nCompany: " + company + "\nPosition: " + position + "\n\nGood luck!\n\n- HTZY Team";
        return send(to, subject, body);
    }

private:
#ifndef _WIN32
    static int forkExec(const std::vector<std::string>& args) {
        pid_t pid = fork();
        if (pid < 0) return -1;
        if (pid == 0) {
            int fd = open("/dev/null", O_WRONLY);
            if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); close(fd); }
            std::vector<const char*> c_args;
            for (auto& a : args) c_args.push_back(a.c_str());
            c_args.push_back(NULL);
            execvp(c_args[0], const_cast<char* const*>(c_args.data()));
            _exit(127);
        }
        int status = 0;
        for (int i = 0; i < 30; i++) {
            pid_t r = waitpid(pid, &status, WNOHANG);
            if (r > 0) return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            usleep(1000000);
        }
        kill(pid, SIGTERM);
        waitpid(pid, &status, 0);
        return -1;
    }
#endif

    static std::string base64Encode(const std::string& input) {
        static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c; valb += 8;
            while (valb >= 0) { out.push_back(tbl[(val >> valb) & 0x3F]); valb -= 6; }
        }
        if (valb > -6) out.push_back(tbl[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }
};
