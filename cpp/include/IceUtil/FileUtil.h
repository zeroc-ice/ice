// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_FILE_UTIL_H
#define ICE_FILE_UTIL_H

#include <IceUtil/Config.h>
#include <sys/stat.h>

namespace IceUtilInternal
{

//
// Detemine if path is an absolute path.
//
ICE_UTIL_API bool isAbsolutePath(const std::string&);

//
// Determine if a file exists.
//
ICE_UTIL_API bool fileExists(const std::string&);

//
// Determine if a directory exists.
//
ICE_UTIL_API bool directoryExists(const std::string&);

#ifdef _WIN32

typedef struct _stat64i32 structstat;
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

//
// OS stat
//
ICE_UTIL_API int stat(const std::string&, structstat*);

};

#endif
