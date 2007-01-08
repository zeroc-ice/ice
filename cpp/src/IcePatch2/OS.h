// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_OS_H
#define ICE_PATCH2_OS_H

#include <Ice/Config.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#ifndef ICE_PATCH2_API
#   ifdef ICE_PATCH2_API_EXPORTS
#       define ICE_PATCH2_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_PATCH2_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace OS
{

#ifdef _WIN32

typedef struct _stat structstat;
#ifdef _MSC_VER
#   define O_RDONLY _O_RDONLY
#   define O_BINARY _O_BINARY

#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#endif

#else

typedef struct stat structstat;
#   define O_BINARY 0

#endif

// BUGFIX: aCC errors if this is stat.
ICE_PATCH2_API int osstat(const std::string&, structstat*);
ICE_PATCH2_API int remove(const std::string&);
ICE_PATCH2_API int rename(const std::string&, const std::string&);
ICE_PATCH2_API int rmdir(const std::string&);

ICE_PATCH2_API int mkdir(const std::string&, int);
ICE_PATCH2_API FILE* fopen(const std::string&, const std::string&);
ICE_PATCH2_API int open(const std::string&, int);
ICE_PATCH2_API int getcwd(std::string&);

}

#endif
