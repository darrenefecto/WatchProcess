#ifdef _WIN32
#include <Windows.h>
#include <TlHelp32.h>

void Windows();
#endif

#ifdef __APPLE__
#include <libproc.h>

void MacOS();
#endif

#ifdef __linux__
#include <iostream>
#include <fstream>
#include <string>

void Linux();
#endif

#include <iostream>
#include <string>

// After App ran
bool AppRan = false;

// When App isn't running
bool notOpen = false;

std::wstring appName;
int main() {

    std::wcout << L"Which Process do you want to watch?" << std::endl;
    std::getline(std::wcin, appName);

    // Check if appName has .exe extension
    std::wstring extension = L".exe";
    if (appName.find(extension) == std::wstring::npos) {
        // Append .exe extension if not found
        appName += extension;
    }

    while (true) {

        #pragma region SystemCheck

        #ifdef _WIN32
                Windows();
        #elif __APPLE__
                MacOS();
        #elif __linux__
                Linux();
        #endif

        #pragma endregion


        // Wait for a second before checking again
        Sleep(1000);
    }

    return 0;
}

#ifdef _WIN32
void Windows()
{
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if App is currently running
    bool AppRun = false;
    if (Process32First(processSnapshot, &processEntry)) {
        do {
            if (_wcsicmp(processEntry.szExeFile, appName.c_str()) == 0) {
                AppRun = true;
                break;
            }
        } while (Process32Next(processSnapshot, &processEntry));
    }

    // Print message if state has changed
    if (AppRun && !AppRan) {
        std::wcout << appName.c_str() << L" has been opened." << std::endl;
        notOpen = false; // Reset not open
    }
    else if (!AppRun && AppRan) {
        std::wcout << appName.c_str() << L" has been closed." << std::endl;
        notOpen = false; // Reset not open
    }
    else if (!AppRun && !notOpen) {
        std::wcout << appName.c_str() << L" is not open." << std::endl;
        notOpen = true; // Set not open
    }

    // Save current state
    AppRan = AppRun;

    // Close process snapshot handle
    CloseHandle(processSnapshot);
}
#endif

#ifdef __APPLE__
void MacOS() {
    // Get the process information for the given process name
    pid_t pid = -1;
    proc_listallpids(NULL, 0);
    struct proc_bsdinfo proc;
    while (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &proc, PROC_PIDTBSDINFO_SIZE)) {
        if (std::wstring(proc.pbi_name) == appName) {
            break;
        }
        pid = proc.pbi_pid;
    }

    // Check if App is currently running
    bool AppRun = (std::wstring(proc.pbi_name) == appName);

    // Print message if state has changed
    if (AppRun && !AppRan) {
        std::wcout << appName.c_str() << L" has been opened." << std::endl;
        notOpen = false; // Reset not open
    }
    else if (!AppRun && AppRan) {
        std::wcout << appName.c_str() << L" has been closed." << std::endl;
        notOpen = false; // Reset not open
    }
    else if (!AppRun && !notOpen) {
        std::wcout << appName.c_str() << L" is not open." << std::endl;
        notOpen = true; // Set not open
    }

    // Save current state
    AppRan = AppRun;
}
#endif

#ifdef __linux__
void Linux() {
    // Open the /proc directory
    DIR* dir = opendir("/proc");
    if (!dir) {
        std::cerr << "Error: Unable to open /proc directory." << std::endl;
        return;
    }

    // Loop through all directories in /proc
    struct dirent* dirEntry;
    while ((dirEntry = readdir(dir)) != NULL) {
        // Check if the directory name is a number (process ID)
        char* endptr;
        long int pid = strtol(dirEntry->d_name, &endptr, 10);
        if (*endptr != '\0') {
            // Directory name is not a number (process ID)
            continue;
        }

        // Open the /proc/[pid]/cmdline file
        std::string cmdlinePath = "/proc/" + std::string(dirEntry->d_name) + "/cmdline";
        std::ifstream cmdlineFile(cmdlinePath.c_str());
        if (!cmdlineFile.is_open()) {
            // Unable to open the cmdline file
            continue;
        }

        // Read the contents of the cmdline file into a string
        std::string cmdline((std::istreambuf_iterator<char>(cmdlineFile)), std::istreambuf_iterator<char>());

        // Check if the cmdline string matches the app name
        std::size_t found = cmdline.find(appName);
        if (found != std::string::npos) {
            // App is running
            if (!AppRan) {
                std::cout << appName << " has been opened." << std::endl;
                notOpen = false; // Reset not open
            }
            AppRan = true;
            closedir(dir);
            return;
        }
    }

    // App is not running
    if (AppRan) {
        std::cout << appName << " has been closed." << std::endl;
        notOpen = false; // Reset not open
    }
    else if (!notOpen) {
        std::cout << appName << " is not open." << std::endl;
        notOpen = true; // Set not open
    }
    AppRan = false;

    // Close the /proc directory
    closedir(dir);
}
#endif