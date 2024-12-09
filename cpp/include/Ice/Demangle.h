// Copyright (c) ZeroC, Inc.

#ifndef ICE_DEMANGLE_H
#define ICE_DEMANGLE_H

#include "Config.h"
#include <string>

namespace IceInternal
{
    /// Demangles a C++ type type.
    /// @param name The possibly mangled type name, as returned by typeid().name().
    /// @return The demangled name.
    ICE_API std::string demangle(const char* name);
}

#endif
