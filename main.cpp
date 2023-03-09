#include <iostream>
#include <optional>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <boost/program_options.hpp>

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include "TlHelp32.h"

namespace po = boost::program_options;

std::string extra_pwsh_script;

void waitUntilProcessExit(DWORD processId) {
    HANDLE hProcHandle = OpenProcess(SYNCHRONIZE, FALSE, processId);
    if (hProcHandle) {
        if (WaitForSingleObject(hProcHandle, INFINITE) == WAIT_OBJECT_0) {
            fmt::print("The process has exited.\n");
            if (!extra_pwsh_script.empty()) {
                auto cmd = fmt::format("powershell.exe -File \"{}\"", extra_pwsh_script);
                fmt::print("{}\n", cmd);
                system(cmd.c_str());
            }
        }
        CloseHandle(hProcHandle);
    }
}

std::optional<DWORD> findProcessByName(std::vector<std::string> const& exes) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            for (auto&& exe : exes) {
                if (stricmp(entry.szExeFile, exe.c_str()) == 0) {
                    return entry.th32ProcessID;
                }
            }
        }
    }
    return {};
}

template <> struct fmt::formatter<po::options_description> : ostream_formatter {};

int main(int argc, char* argv[]) {// Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("target_exes,t", po::value<std::vector<std::string>>()->multitoken(), "set targeting executables")
            ("script,s", po::value<std::string>(), "Powershell Script to execute when exe stops")
            ;

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            fmt::print("{}\n", desc);
            return 1;
        }

        if (vm.count("script")) {
            extra_pwsh_script = vm["script"].as<std::string>();
        }

        if (vm.count("target_exes")) {
            auto exes = vm["target_exes"].as<std::vector<std::string>>();
            fmt::print("targets: {}\n", exes);

            while (true) {
                auto process = findProcessByName(exes);
                if (process) {
                    fmt::print("process found! Wait for it to stop\n");
                    waitUntilProcessExit(process.value());
                }
                else {
                    Sleep(30 * 60 * 1'000);
                }
            }
        }
        else {
            fmt::print("{}\n", desc);
            return 1;
        }
    }
    catch (...) {
        fmt::print("{}\n", desc);
    }
}