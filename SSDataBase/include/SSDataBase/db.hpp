#pragma once

#ifdef _WIN32
    #define SH_LIB __declspec(dllexport)
#else
    #define SH_LIB
#endif

class SH_LIB database {
public:
    database();
};