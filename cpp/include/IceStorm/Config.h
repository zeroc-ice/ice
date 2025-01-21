// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_CONFIG_H
#define ICESTORM_CONFIG_H

#include "Ice/Config.h"

#if defined(_MSC_VER) && !defined(ICESTORM_API_EXPORTS)
#    pragma comment(lib, ICE_LIBNAME("IceStorm")) // Automatically link with IceStorm[D].lib
#endif

#endif
