// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_CONFIG_H
#define DATASTORM_CONFIG_H

#include "Ice/Config.h"

#ifndef DATASTORM_API
#    if defined(DATASTORM_API_EXPORTS)
#        define DATASTORM_API ICE_DECLSPEC_EXPORT
#    else
#        define DATASTORM_API ICE_DECLSPEC_IMPORT
#    endif
#endif

#if defined(_MSC_VER) && !defined(DATASTORM_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("DataStorm")) // Automatically link with DataStorm[D].lib
#endif

#endif
