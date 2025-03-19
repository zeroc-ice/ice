// Copyright (c) ZeroC, Inc.

#ifndef ICE_FILE_UTIL_H
#define ICE_FILE_UTIL_H

#include "Ice/Config.h"
#include "Ice/LocalException.h"

#ifdef _WIN32
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>

namespace IceInternal
{
#ifdef _WIN32
    const char* const separator = "\\";
#else
    const char* const separator = "/";
#endif

    //
    // Determine if path is an absolute path.
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

#    ifdef _MSC_VER
#        ifndef O_RDONLY
#            define O_RDONLY _O_RDONLY
#        endif

#        ifndef O_BINARY
#            define O_BINARY _O_BINARY
#        endif

#        ifndef S_ISDIR
#            define S_ISDIR(mode) ((mode) & _S_IFDIR)
#        endif

#        ifndef S_ISREG
#            define S_ISREG(mode) ((mode) & _S_IFREG)
#        endif
#    endif

#else

    using structstat = struct stat;
#    define O_BINARY 0

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
    class ICE_API FileLock
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

        FileLock(const FileLock&) = delete;

        //
        // The destructor releases the lock and removes the file.
        //
        virtual ~FileLock();

        FileLock& operator=(const FileLock&) = delete;

    private:
#ifdef _WIN32
        HANDLE _fd{INVALID_HANDLE_VALUE};
#else
        int _fd{-1};
#endif
        std::string _path;
    };

    /// This exception indicates the failure to lock a file.
    class ICE_API FileLockException final : public Ice::LocalException
    {
    public:
        /// Constructs a FileLockException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param error The error code.
        /// @param path The path of the file.
        FileLockException(const char* file, int line, int error, const std::string& path);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    // Use streamFilename to construct the filename given to std stream classes
    // like ifstream and ofstream.
#if defined(_WIN32)
    ICE_API std::wstring streamFilename(const std::string&);
#else
    inline std::string streamFilename(const std::string& filename) { return filename; }
#endif
}
#endif
