//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_FILE_UTIL_H
#define ICE_FILE_UTIL_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

namespace IceUtilInternal
{

extern const ICE_API std::string pathsep;
extern const ICE_API std::string separator;

//
// Detemine if path is an absolute path.
//
ICE_API bool isAbsolutePath(const std::string&);

//
// Determine if a file exists.
//
ICE_API bool fileExists(const std::string&);

//
// Determine if a directory exists.
//
ICE_API bool directoryExists(const std::string&);

//
// Determine if a directory exists and is empty.
//
ICE_API bool isEmptyDirectory(const std::string&);

#ifdef _WIN32

typedef struct _stat64i32 structstat;

#ifdef _MSC_VER
#ifndef O_RDONLY
#   define O_RDONLY _O_RDONLY
#endif

#ifndef O_BINARY
#   define O_BINARY _O_BINARY
#endif

#ifndef S_ISDIR
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#endif

#ifndef S_ISREG
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#endif
#endif

#else

typedef struct stat structstat;
#   define O_BINARY 0

#endif

//
// OS stat
//
ICE_API int stat(const std::string&, structstat*);
ICE_API int remove(const std::string&);
ICE_API int rename(const std::string&, const std::string&);
ICE_API int rmdir(const std::string&);

ICE_API int mkdir(const std::string&, int);
ICE_API FILE* fopen(const std::string&, const std::string&);
ICE_API FILE* freopen(const std::string&, const std::string&, FILE*);
ICE_API int open(const std::string&, int);
ICE_API int getcwd(std::string&);

ICE_API int unlink(const std::string&);
ICE_API int close(int);

//
// This class is used to implement process file locking. This class
// is not intended to do file locking within the same process.
//
class ICE_API FileLock : public IceUtil::Shared, public IceUtil::noncopyable
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

//
// Use streamFilename to construct the filename given to std stream classes
// like ifstream and ofstream.
//
#ifdef _WIN32
ICE_API std::wstring streamFilename(const std::string&);
#else
inline std::string streamFilename(const std::string& filename)
{
    return filename;
}
#endif

}
#endif
