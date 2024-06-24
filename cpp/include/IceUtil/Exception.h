//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include "Config.h"

#include <exception>
#include <ostream>
#include <vector>

namespace IceUtil
{
    /**
     * Abstract base class for all Ice exceptions. Use the Ice::Exception alias instead
     * of IceUtil::Exception.
     * \headerfile Ice/Ice.h
     */
    class ICE_API Exception : public std::exception
    {
    public:
        /**
         * Returns a description of this exception.
         * @return The description.
         */
        const char* what() const noexcept override;

        /**
         * Returns the type ID of this exception. This corresponds to the Slice
         * type ID for Slice-defined exceptions, and to a similar fully scoped name
         * for other exceptions. For example "::IceUtil::SyscallException".
         * @return The type ID of this exception
         */
        virtual const char* ice_id() const noexcept = 0;

        /**
         * Outputs a description of this exception to a stream.
         * @param os The output stream.
         */
        virtual void ice_print(std::ostream& os) const;

        /**
         * Returns the name of the file where this exception was constructed.
         * @return The file name.
         */
        const char* ice_file() const noexcept;

        /**
         * Returns the line number where this exception was constructed.
         * @return The line number.
         */
        int ice_line() const noexcept;

        /**
         * Returns the stack trace at the point this exception was constructed
         * @return The stack trace as a string.
         */
        std::string ice_stackTrace() const;

    protected:
        /**
         * Constructs an exception.
         * @param message The message returned by what().
         * @param file The file where this exception is constructed.
         * @param line The line where this exception is constructed.
         */
        Exception(const char* message, const char* file, int line) noexcept;

        /**
         * Constructs an exception.
         * @param message The message adopted by this exception and returned by what().
         * @param file The file where this exception is constructed.
         * @param line The line where this exception is constructed.
         */
        Exception(std::string&& message, const char* file, int line) noexcept;

        /**
         * Constructs an exception without a message. Exception's implementation of what() returns ice_id().
         * @param file The file where this exception is constructed.
         * @param line The line where this exception is constructed.
         */
        Exception(const char* file, int line) noexcept
            : Exception(nullptr, file, line)
        {
        }

    private:
        const std::string _whatString; // optional storage for _what
        const char* _what;             // can be nullptr
        const char* _file;             // can be nullptr
        const int _line;
        const std::vector<void*> _stackFrames;
    };

    ICE_API std::ostream& operator<<(std::ostream&, const Exception&);

    /**
     * This exception indicates the failure of a string conversion.
     * \headerfile Ice/Ice.h
     */
    class ICE_API IllegalConversionException : public Exception
    {
    public:
        IllegalConversionException(const char*, int, std::string) noexcept;

        IllegalConversionException(const char* file, int line) noexcept
            : Exception(file, line)
        {
        }

        const char* ice_id() const noexcept override;
        void ice_print(std::ostream&) const override;

        /**
         * Provides the reason this exception was thrown.
         * @return The reason.
         */
        std::string reason() const noexcept;

    private:
        const std::string _reason;
    };

    /**
     * This exception indicates the failure to lock a file.
     * \headerfile Ice/Ice.h
     */
    class ICE_API FileLockException : public Exception
    {
    public:
        FileLockException(const char*, int, int, std::string) noexcept;

        const char* ice_id() const noexcept override;
        void ice_print(std::ostream&) const override;

        /**
         * Returns the path to the file.
         * @return The file path.
         */
        const std::string& path() const noexcept;

        /**
         * Returns the error number for the failed locking attempt.
         * @return The error number.
         */
        int error() const noexcept;

    private:
        const int _error;
        std::string _path;
    };
}

namespace IceUtilInternal
{
    enum StackTraceImpl
    {
        STNone,
        STDbghelp,
        STLibbacktrace,
        STLibbacktracePlus,
        STBacktrace
    };

    ICE_API StackTraceImpl stackTraceImpl();
}

#endif
