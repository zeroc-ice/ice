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

//
// Base class for all Ice run-time exceptions
//
class ICE_API LocalException : public IceUtil::Exception
{
public:

    LocalException(const char*, int);

#ifdef ICE_CPP11_COMPILER
    LocalException(const LocalException&) = default;
    virtual ~LocalException();
#else
    virtual ~LocalException() throw();
#endif

#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<LocalException> ice_clone() const;
#else
    virtual LocalException* ice_clone() const = 0;
#endif

    static const std::string& ice_staticId();
};

//
// Base class for all Ice user exceptions
//
class ICE_API UserException : public IceUtil::Exception
{
public:

#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<UserException> ice_clone() const;
#else
    virtual UserException* ice_clone() const = 0;
#endif
    virtual Ice::SlicedDataPtr ice_getSlicedData() const;

    static const std::string& ice_staticId();

    virtual void _write(::Ice::OutputStream*) const;
    virtual void _read(::Ice::InputStream*);

    virtual bool _usesClasses() const;

protected:

    virtual void _writeImpl(::Ice::OutputStream*) const {}
    virtual void _readImpl(::Ice::InputStream*) {}
};

//
// Base class for all Ice system exceptions
//
// System exceptions are currently Ice internal, non-documented
// exceptions.
//
class ICE_API SystemException : public IceUtil::Exception
{
public:

    SystemException(const char*, int);
#ifdef ICE_CPP11_COMPILER
    SystemException(const SystemException&) = default;
    virtual ~SystemException();
#else
    virtual ~SystemException() throw();
#endif

#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<SystemException> ice_clone() const;
#else
    virtual SystemException* ice_clone() const = 0;
#endif

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
