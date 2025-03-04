// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_CONFIG_H
#define ICEGRID_CONFIG_H

// This header is included in all IceGrid generated headers.

#include "Ice/Config.h"

#if defined(_MSC_VER) && !defined(ICEGRID_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceGrid")) // Automatically link with IceGrid[D].lib
#endif

#endif
