#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <climits>
#else
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#endif

#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/settings.h"
#include "main.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

using namespace crashpad;
using namespace std;

// Function to get the executable directory
std::string getExecutableDir() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string pathStr(path);
    size_t lastBackslash = pathStr.find_last_of('\\');
    if (lastBackslash != std::string::npos) {
        return pathStr.substr(0, lastBackslash);
    }
    return "";
#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        std::string pathStr(path);
        size_t lastSlash = pathStr.find_last_of('/');
        if (lastSlash != std::string::npos) {
            return pathStr.substr(0, lastSlash);
        }
    }
    return "";
#else // Linux
    char pBuf[FILENAME_MAX];
    int len = sizeof(pBuf);
    int bytes = MIN(readlink("/proc/self/exe", pBuf, len), len - 1);
    if (bytes >= 0) {
        pBuf[bytes] = '\0';
    }

    char *lastForwardSlash = strrchr(&pBuf[0], '/');
    if (lastForwardSlash == NULL)
        return "";
    *lastForwardSlash = '\0';

    return pBuf;
#endif
}

// Function to initialize Crashpad with BugSplat integration
bool initializeCrashpad(std::string dbName, std::string appName, std::string appVersion) {
    using namespace crashpad;
    
    // Get directory where the exe lives
    std::string exeDir = getExecutableDir();

    // Ensure that crashpad_handler is shipped with your application
#ifdef _WIN32
    base::FilePath handler(base::FilePath::StringType(exeDir.begin(), exeDir.end()) + L"/crashpad_handler.exe");
#else
    base::FilePath handler(exeDir + "/crashpad_handler");
#endif

    // Directory where reports and metrics will be saved
#ifdef _WIN32
    base::FilePath reportsDir(base::FilePath::StringType(exeDir.begin(), exeDir.end()));
    base::FilePath metricsDir(base::FilePath::StringType(exeDir.begin(), exeDir.end()));
#else
    base::FilePath reportsDir(exeDir);
    base::FilePath metricsDir(exeDir);
#endif

    // Configure url with your BugSplat database
    std::string url = "https://" + dbName + ".bugsplat.com/post/bp/crash/crashpad.php";

    // Metadata that will be posted to BugSplat
    std::map<std::string, std::string> annotations;
    annotations["format"] = "minidump";              // Required: Crashpad setting to save crash as a minidump
    annotations["database"] = dbName;                // Required: BugSplat database
    annotations["product"] = appName;                // Required: BugSplat appName
    annotations["version"] = appVersion;             // Required: BugSplat appVersion
    annotations["key"] = "Sample key";               // Optional: BugSplat key field
    annotations["user"] = "fred@bugsplat.com";       // Optional: BugSplat user email
    annotations["list_annotations"] = "Sample crash from dynamic library"; // Optional: BugSplat crash description

    // Disable crashpad rate limiting
    std::vector<std::string> arguments;
    arguments.push_back("--no-rate-limit");

    // File paths of attachments to be uploaded with the minidump file at crash time
    std::vector<base::FilePath> attachments;
#ifdef _WIN32
    base::FilePath attachment(base::FilePath::StringType(exeDir.begin(), exeDir.end()) + L"/attachment.txt");
#else
    base::FilePath attachment(exeDir + "/attachment.txt");
#endif
    attachments.push_back(attachment);

    // Initialize Crashpad database
    std::unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(reportsDir);
    if (database == nullptr) {
        return false;
    }

    // Enable automated crash uploads
    Settings* settings = database->GetSettings();
    if (settings == nullptr) {
        return false;
    }
    settings->SetUploadsEnabled(true);

    // Start crash handler
    CrashpadClient client;
    bool success = client.StartHandler(
        handler,
        reportsDir,
        metricsDir,
        url,
        annotations,
        arguments,
        true,  // Restartable
        true,  // Asynchronous
        attachments  // Add attachment
    );

    return success;
}

// Create some dummy frames for a more interesting call stack
void func2() {
    std::cout << "In func2, loading library and about to crash...\n";

    // Get the executable directory to find our library
    std::string exeDir = getExecutableDir();
    
#ifdef _WIN32
    std::string libPath = exeDir + "/crash.dll";
    // Load the DLL
    HMODULE handle = LoadLibraryA(libPath.c_str());
    if (!handle) {
        std::cerr << "Failed to load library: " << GetLastError() << std::endl;
        return;
    }

    // Get the crash function
    typedef void (*crash_func_t)(void);
    crash_func_t crash_func = (crash_func_t)GetProcAddress(handle, "crash");
    if (!crash_func) {
        std::cerr << "Failed to get crash function: " << GetLastError() << std::endl;
        FreeLibrary(handle);
        return;
    }
#elif defined(__APPLE__)
    std::string libPath = exeDir + "/libcrash.dylib";
    // Load the shared library
    void *handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load library: " << dlerror() << std::endl;
        return;
    }

    // Get the crash function
    typedef void (*crash_func_t)(void);
    dlerror(); // Clear any existing error
    crash_func_t crash_func = (crash_func_t)dlsym(handle, "crash");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Failed to get crash function: " << dlsym_error << std::endl;
        dlclose(handle);
        return;
    }
#else // Linux
    std::string libPath = exeDir + "/libcrash.so.2";
    // Load the shared library
    void *handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load library: " << dlerror() << std::endl;
        return;
    }

    // Get the crash function
    typedef void (*crash_func_t)(void);
    dlerror(); // Clear any existing error
    crash_func_t crash_func = (crash_func_t)dlsym(handle, "crash");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Failed to get crash function: " << dlsym_error << std::endl;
        dlclose(handle);
        return;
    }
#endif

    // Call the crash function
    crash_func();

    // We should never reach here
#ifdef _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}

void func1() {
    std::cout << "In func1, calling func2...\n";
    func2();
}

void func0() {
    std::cout << "In func0, calling func1...\n";
    func1();
}

void generateExampleCallstackAndCrash() {
    std::cout << "Starting call chain...\n";
    func0();
}

int main() {
    // Initialize Crashpad
    if (!initializeCrashpad(BUGSPLAT_DATABASE, BUGSPLAT_APP_NAME, BUGSPLAT_APP_VERSION)) {
        std::cerr << "Failed to initialize Crashpad" << std::endl;
        return 1;
    }

    std::cout << "Hello, World!" << std::endl;
    std::cout << "Crashpad initialized successfully!" << std::endl;
    std::cout << "Generating crash..." << std::endl;
    
    // Generate an example callstack and crash
    generateExampleCallstackAndCrash();
    
    return 0;
} 