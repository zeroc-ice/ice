// Copyright (c) ZeroC, Inc.

#include "Ice/Demangle.h"

#if defined(__GNUC__) || defined(__clang__)
#    include <cxxabi.h>
#endif

using namespace std;

string
IceInternal::demangle(const char* name)
{
#if defined(__GNUC__) || defined(__clang__)
    int status;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if (status == 0) // success
    {
        string result{demangled};
        std::free(demangled);
        return result;
    }
#endif
    return name; // keep the original name
}
