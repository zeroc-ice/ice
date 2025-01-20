// Copyright (c) ZeroC, Inc.

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include "Config.h"
#include "ValueF.h"

#include <exception>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace Ice
{
    /**
     * Abstract base class for all Ice exceptions. It has only two derived classes: LocalException and UserException.
     * \headerfile Ice/Ice.h
     */
    class ICE_API Exception : public std::exception
    {
    public:
        /**
         * Constructs an exception with a default message.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         */
        Exception(const char* file, int line) noexcept;

        /**
         * Constructs an exception.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The error message adopted by this exception and returned by what().
         */
        Exception(const char* file, int line, std::string message);

        /**
         * Copy constructor.
         * @param other The exception to copy.
         */
        Exception(const Exception& other) noexcept = default;

        /**
         * Assignment operator.
         * @param rhs The exception to assign.
         */
        Exception& operator=(const Exception& rhs) noexcept = default;

        /**
         * Returns the error message of this exception.
         * @return The error message.
         */
        [[nodiscard]] const char* what() const noexcept override;

        /**
         * Returns the type ID of this exception. This corresponds to the Slice type ID for Slice-defined exceptions,
         * and to a similar fully scoped name for other exceptions. For example "::Ice::CommunicatorDestroyedException".
         * @return The type ID of this exception
         */
        [[nodiscard]] virtual const char* ice_id() const noexcept = 0;

        /**
         * Outputs a description of this exception to a stream. This function is called by operator<<(std::ostream&,
         * const Ice::Exception&). The default implementation outputs ice_id(). The application can override the
         * ice_print of a user exception to produce a more detailed description, with typically the ice_id() plus
         * additional information.
         * @param os The output stream.
         */
        virtual void ice_print(std::ostream& os) const { os << ice_id(); }

        /**
         * Returns the name of the file where this exception was constructed.
         * @return The file name.
         */
        [[nodiscard]] const char* ice_file() const noexcept;

        /**
         * Returns the line number where this exception was constructed.
         * @return The line number.
         */
        [[nodiscard]] int ice_line() const noexcept;

        /**
         * Returns the stack trace at the point this exception was constructed.
         * @return The stack trace as a string, or an empty string if stack trace collection is not enabled.
         */
        [[nodiscard]] std::string ice_stackTrace() const;

        /**
         * Enables the collection of stack traces for exceptions. On Windows, calling this function more than once is
         * useful to refresh the symbol module list; on other platforms, the second and subsequent calls have no effect.
         */
        static void ice_enableStackTraceCollection();

    private:
        friend ICE_API std::ostream& operator<<(std::ostream&, const Exception&);

        const char* _file;                                // can be nullptr
        int _line;                                        // not used when _file is nullptr
        std::shared_ptr<std::string> _whatString;         // shared storage for custom _what message.
        const char* _what;                                // can be nullptr
        std::shared_ptr<std::vector<void*>> _stackFrames; // shared storage for stack frames.
    };

    ICE_API std::ostream& operator<<(std::ostream&, const Exception&);
}

#endif
