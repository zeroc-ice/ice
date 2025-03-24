// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOCAL_EXCEPTION_H
#define ICE_LOCAL_EXCEPTION_H

#include "Exception.h"

#include <memory>
#include <string>
#include <vector>

namespace Ice
{
    /// Base class for all Ice exceptions not defined in Slice.
    /// @headerfile Ice/Ice.h
    class ICE_API LocalException : public Exception
    {
    public:
        /// Constructs a local exception.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The error message adopted by this exception and returned by what().
        LocalException(const char* file, int line, std::string message);

        /// Gets the error message of this local Ice exception.
        /// @return The error message.
        [[nodiscard]] const char* what() const noexcept final;

        void ice_print(std::ostream& os) const final;

        [[nodiscard]] const char* ice_id() const noexcept override;

        /// Gets the name of the file where this exception was constructed.
        /// @return The file name.
        [[nodiscard]] const char* ice_file() const noexcept;

        /// Gets the line number where this exception was constructed.
        /// @return The line number.
        [[nodiscard]] int ice_line() const noexcept;

        /// Gets the stack trace at the point this exception was constructed.
        /// @return The stack trace as a string, or an empty string if stack trace collection is not enabled.
        [[nodiscard]] std::string ice_stackTrace() const;

        /// Enables the collection of stack traces for exceptions. On Windows, calling this function more than once is
        /// useful to refresh the symbol module list; on other platforms, the second and subsequent calls have no
        /// effect.
        static void ice_enableStackTraceCollection();

    private:
        const char* _file;
        int _line;
        std::shared_ptr<std::string> _whatString;         // shared storage for custom _what message.
        std::shared_ptr<std::vector<void*>> _stackFrames; // shared storage for stack frames.
    };
}

#endif
