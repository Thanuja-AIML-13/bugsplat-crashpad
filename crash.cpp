#include <cstddef>

extern "C" {
    // Function that will cause a crash
    #ifdef _WIN32
    __declspec(dllexport)
    #else
    __attribute__((visibility("default")))
    #endif
    void crash() {
        // Dereference null pointer to cause a crash
        *(volatile int*)nullptr = 42;
    }
} 