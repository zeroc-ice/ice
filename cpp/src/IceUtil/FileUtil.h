// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_FILE_UTIL_H
#define ICE_FILE_UTIL_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <fstream>

namespace IceUtilInternal
{

extern const ICE_UTIL_API std::string pathsep;
extern const ICE_UTIL_API std::string separator;

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

//
// Determine if a directory exists and is empty.
//
ICE_UTIL_API bool isEmptyDirectory(const std::string&);

#ifdef _WIN32

#if defined(__MINGW32__)
typedef struct _stat structstat;
#else
typedef struct _stat64i32 structstat;
#endif

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
ICE_UTIL_API int remove(const std::string&);
ICE_UTIL_API int rename(const std::string&, const std::string&);
ICE_UTIL_API int rmdir(const std::string&);

ICE_UTIL_API int mkdir(const std::string&, int);
ICE_UTIL_API FILE* fopen(const std::string&, const std::string&);
ICE_UTIL_API FILE* freopen(const std::string&, const std::string&, FILE*);
ICE_UTIL_API int open(const std::string&, int);

#ifndef ICE_OS_WINRT
ICE_UTIL_API int getcwd(std::string&);
#endif

ICE_UTIL_API int unlink(const std::string&);
ICE_UTIL_API int close(int);

//
// This class is used to implement process file locking. This class
// is not intended to do file locking within the same process.
//
class ICE_UTIL_API FileLock : public IceUtil::Shared, public IceUtil::noncopyable
{
public:
    //
    // The constructor opens the given file (eventually creating it)
    // and acquires a lock on the file or throws FileLockException if
    // the file couldn't be locked.
    //
    // If the lock can be acquired, the process pid is written to the
    // file.
    //
    FileLock(const std::string&);

    //
    // The destructor releases the lock and removes the file.
    //
    virtual ~FileLock();

private:

#ifdef _WIN32
    HANDLE _fd;
#else
    int _fd;
#endif
    std::string _path;
};

typedef IceUtil::Handle<FileLock> FileLockPtr;

class ICE_UTIL_API ifstream : public std::ifstream
{
public:

    ifstream();
    ifstream(const std::string&, std::ios_base::openmode mode = std::ios_base::in);
    void open(const std::string&, std::ios_base::openmode mode = std::ios_base::in);

#ifdef __SUNPRO_CC
    using std::ifstream::open;
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    ifstream(const ifstream&) = delete;
#endif

private:

    // Hide const char* definitions since they shouldn't be used.
    ifstream(const char*);
    void open(const char*, std::ios_base::openmode mode = std::ios_base::in);
};

class ICE_UTIL_API ofstream : public std::ofstream
{
public:

    ofstream();
    ofstream(const std::string&, std::ios_base::openmode mode = std::ios_base::out);
    void open(const std::string&, std::ios_base::openmode mode = std::ios_base::out);

#ifdef __SUNPRO_CC
    using std::ofstream::open;
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    ofstream(const ofstream&) = delete;
#endif

private:

    // Hide const char* definitions since they shouldn't be used.
    ofstream(const char*);
    void open(const char*, std::ios_base::openmode mode = std::ios_base::out);
};

//
// Use streamFilename to construct the filename given to std stream classes
// like ifstream and ofstream.
//
#if defined(_WIN32) && !defined(__MINGW32__)
ICE_UTIL_API std::wstring streamFilename(const std::string&);
#else
inline std::string streamFilename(const std::string& filename)
{
    return filename;
}
#endif

}
#endif
