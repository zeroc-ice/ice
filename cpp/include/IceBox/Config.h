// Copyright (c) ZeroC, Inc.

#ifndef ICEBOX_CONFIG_H
#define ICEBOX_CONFIG_H

// This header is included in all IceBox generated headers.

#include "Ice/Config.h"

#if defined(_MSC_VER) && !defined(ICEBOX_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceBox")) // Automatically link with IceBox[D].lib
#endif

#endif
