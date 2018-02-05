// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <Ice/Config.h>
#include <Ice/Format.h>
#include <Ice/Handle.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/SlicedDataF.h>

namespace Ice
{

class OutputStream;
class InputStream;

typedef IceUtil::Exception Exception;

/**
 * Base class for all Ice run-time exceptions.
 * \headerfile Ice/Ice.h
 */
class ICE_API LocalException : public IceUtil::Exception
{
public:

    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    LocalException(const char* file, int line);

#ifdef ICE_CPP11_COMPILER
    LocalException(const LocalException&) = default;
    virtual ~LocalException();
#else
    virtual ~LocalException() throw();
#endif

    /**
     * Polymporphically clones this exception.
     * @return A shallow copy of this exception.
     */
#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<LocalException> ice_clone() const;
#else
    virtual LocalException* ice_clone() const = 0;
#endif

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    static const std::string& ice_staticId();
};

/**
 * Base class for all Ice user exceptions.
 * \headerfile Ice/Ice.h
 */
class ICE_API UserException : public IceUtil::Exception
{
public:

    /**
     * Polymporphically clones this exception.
     * @return A shallow copy of this exception.
     */
#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<UserException> ice_clone() const;
#else
    virtual UserException* ice_clone() const = 0;
#endif
    virtual Ice::SlicedDataPtr ice_getSlicedData() const;

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    static const std::string& ice_staticId();

    /// \cond STREAM
    virtual void _write(::Ice::OutputStream*) const;
    virtual void _read(::Ice::InputStream*);

    virtual bool _usesClasses() const;
    /// \endcond

protected:

    /// \cond STREAM
    virtual void _writeImpl(::Ice::OutputStream*) const {}
    virtual void _readImpl(::Ice::InputStream*) {}
    /// \endcond
};

/**
 * Base class for all Ice system exceptions.
 *
 * System exceptions are currently Ice internal, non-documented
 * exceptions.
 * \headerfile Ice/Ice.h
 */
class ICE_API SystemException : public IceUtil::Exception
{
public:

    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    SystemException(const char* file, int line);

#ifdef ICE_CPP11_COMPILER
    SystemException(const SystemException&) = default;
    virtual ~SystemException();
#else
    virtual ~SystemException() throw();
#endif

    /**
     * Polymporphically clones this exception.
     * @return A shallow copy of this exception.
     */
#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<SystemException> ice_clone() const;
#else
    virtual SystemException* ice_clone() const = 0;
#endif

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    static const std::string& ice_staticId();
};

}

namespace IceInternal
{

namespace Ex
{

ICE_API void throwUOE(const ::std::string&, const ::Ice::ValuePtr&);
ICE_API void throwMemoryLimitException(const char*, int, size_t, size_t);
ICE_API void throwMarshalException(const char*, int, const std::string&);

}

}

#endif
