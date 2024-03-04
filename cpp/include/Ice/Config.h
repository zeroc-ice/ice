//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONFIG_H
#define ICE_CONFIG_H

#include <IceUtil/Config.h>

//
// Some include files we need almost everywhere
//
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#if defined(_WIN32)
#    include <process.h>
#else
#    include <sys/types.h>
#    include <cstddef>
#endif

#ifdef ICE_SWIFT
#   include <dispatch/dispatch.h>
#endif

#endif
