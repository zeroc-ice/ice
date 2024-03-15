//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include <IceUtil/Config.h>

#include <exception>
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
         * Default constructor. Equivalent to Exception(nullptr, 0).
         */
        Exception() noexcept;

        /**
         * Constructs the exception.
         * @param file The file where this exception is constructed.
         * @param line The line where this exception is constructed.
         */
        Exception(const char* file, int line) noexcept;

        /**
         * Returns the type ID of this exception. This corresponds to the Slice
         * type ID for Slice-defined exceptions, and to a similar fully scoped name
         * for other exceptions. For example "::IceUtil::SyscallException".
         * @return The type ID of this exception
         */
        virtual std::string ice_id() const = 0;

        /**
         * Outputs a description of this exception to a stream.
         * @param os The output stream.
         */
        virtual void ice_print(std::ostream& os) const;

        /**
         * Returns a description of this exception.
         * @return The description.
         */
        virtual const char* what() const noexcept;

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

    private:
        const char* _file;
        int _line;
        const std::vector<void*> _stackFrames;
        mutable ::std::string _str; // Initialized lazily in what().
    };

    ICE_API std::ostream& operator<<(std::ostream&, const Exception&);

    /**
     * This exception indicates that a function was called with an illegal parameter
     * value. It is used only by the Slice to C++98 mapping; std::invalid_argument is
     * used by the Slice to C++11 mapping.
     * \headerfile Ice/Ice.h
     */
    class ICE_API IllegalArgumentException : public Exception
    {
    public:
        using Exception::Exception;
        IllegalArgumentException(const char*, int, std::string) noexcept;

        std::string ice_id() const override;
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
     * This exception indicates the failure of a string conversion.
     * \headerfile Ice/Ice.h
     */
    class ICE_API IllegalConversionException : public Exception
    {
    public:
        using Exception::Exception;
        IllegalConversionException(const char*, int, std::string) noexcept;

        std::string ice_id() const override;
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

        std::string ice_id() const override;
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
