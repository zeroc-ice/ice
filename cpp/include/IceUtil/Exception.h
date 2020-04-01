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
     * Constructs the exception. Equivalent to Exception(nullptr, 0).
     */
    Exception();

    /**
     * Constructs the exception.
     * @param file The file where this exception is constructed.
     * @param line The line where this exception is constructed.
     */
    Exception(const char* file, int line);

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
     * Returns a shallow polymorphic copy of this exception.
     * @return A unique_ptr to the new shallow copy.
     */
    std::unique_ptr<Exception> ice_clone() const;

    /**
     * Throws this exception.
     */
    virtual void ice_throw() const = 0;

    /**
     * Returns the name of the file where this exception was constructed.
     * @return The file name.
     */
    const char* ice_file() const;

    /**
     * Returns the line number where this exception was constructed.
     * @return The line number.
     */
    int ice_line() const;

    /**
     * Returns the stack trace at the point this exception was constructed
     * @return The stack trace as a string.
     */
    std::string ice_stackTrace() const;

protected:

    /// \cond INTERNAL
    virtual Exception* ice_cloneImpl() const = 0;
    /// \endcond

private:

    const char* _file;
    int _line;
    const std::vector<void*> _stackFrames;
    mutable ::std::string _str; // Initialized lazily in what().
};

ICE_API std::ostream& operator<<(std::ostream&, const Exception&);

/**
 * Helper template for the implementation of Ice::Exception.
 * It implements ice_clone and ice_throw.
 * \headerfile Ice/Ice.h
 */
template<typename E, typename B = Exception>
class ExceptionHelper : public B
{
public:

    using B::B;

    std::unique_ptr<E> ice_clone() const
    {
        return std::unique_ptr<E>(static_cast<E*>(ice_cloneImpl()));
    }

    virtual void ice_throw() const override
    {
        throw static_cast<const E&>(*this);
    }

protected:

    /// \cond INTERNAL
    virtual Exception* ice_cloneImpl() const override
    {
        return new E(static_cast<const E&>(*this));
    }
    /// \endcond
};

/**
 * This exception indicates an attempt to dereference a null IceUtil::Handle or
 * IceInternal::Handle.
 * \headerfile Ice/Ice.h
 */
class ICE_API NullHandleException : public ExceptionHelper<NullHandleException>
{
public:

    NullHandleException(const char*, int);
    virtual std::string ice_id() const;

};

/**
 * This exception indicates that a function was called with an illegal parameter
 * value. It is used only by the Slice to C++98 mapping; std::invalid_argument is
 * used by the Slice to C++11 mapping.
 * \headerfile Ice/Ice.h
 */
class ICE_API IllegalArgumentException : public ExceptionHelper<IllegalArgumentException>
{
public:

    IllegalArgumentException(const char*, int);
    IllegalArgumentException(const char*, int, const std::string&);

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

    /**
     * Provides the reason this exception was thrown.
     * @return The reason.
     */
    std::string reason() const;

private:

    const std::string _reason;
};

/**
 * This exception indicates the failure of a string conversion.
 * \headerfile Ice/Ice.h
 */
class ICE_API IllegalConversionException : public ExceptionHelper<IllegalConversionException>
{
public:

    IllegalConversionException(const char*, int);
    IllegalConversionException(const char*, int, const std::string&);

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

    /**
     * Provides the reason this exception was thrown.
     * @return The reason.
     */
    std::string reason() const;

private:

    const std::string _reason;
};

/**
 * This exception indicates the failure of a system call.
 * \headerfile Ice/Ice.h
 */
class ICE_API SyscallException : public ExceptionHelper<SyscallException>
{
public:

    SyscallException(const char*, int, int);

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

    /**
     * Provides the error number returned by the system call.
     * @return The error number.
     */
    int error() const;

private:

    const int _error;
};

template<typename E>
using SyscallExceptionHelper = ExceptionHelper<E, SyscallException>;

/**
 * This exception indicates the failure to lock a file.
 * \headerfile Ice/Ice.h
 */
class ICE_API FileLockException : public ExceptionHelper<FileLockException>
{
public:

    FileLockException(const char*, int, int, const std::string&);

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

    /**
     * Returns the path to the file.
     * @return The file path.
     */
    std::string path() const;

    /**
     * Returns the error number for the failed locking attempt.
     * @return The error number.
     */
    int error() const;

private:

    const int _error;
    std::string _path;
};

/**
 * This exception indicates an IceUtil::Optional is not set.
 * Used only by the Slice to C++98 mapping.
 * \headerfile Ice/Ice.h
 */
class ICE_API OptionalNotSetException : public ExceptionHelper<OptionalNotSetException>
{
public:

    OptionalNotSetException(const char*, int);
    virtual std::string ice_id() const;

};

}

namespace IceUtilInternal
{

enum StackTraceImpl { STNone, STDbghelp, STLibbacktrace, STLibbacktracePlus, STBacktrace };

ICE_API StackTraceImpl stackTraceImpl();

}

#endif
