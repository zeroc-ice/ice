// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

namespace IceInternal
{

namespace Ex
{

ICE_API void throwUOE(const ::std::string&, const ::Ice::ValuePtr&);
ICE_API void throwMemoryLimitException(const char*, int, size_t, size_t);
ICE_API void throwMarshalException(const char*, int, const std::string&);

}

}

namespace Ice
{

class OutputStream;
class InputStream;

typedef IceUtil::Exception Exception;

class ICE_API LocalException : public IceUtil::Exception
{
public:

    LocalException(const char*, int);
    virtual ~LocalException() ICE_NOEXCEPT;

    virtual std::string ice_id() const = 0;
#ifndef ICE_CPP11_MAPPING
    ICE_DEPRECATED_API("ice_name() is deprecated, use ice_id() instead.")
    virtual std::string ice_name() const = 0;
    
    virtual LocalException* ice_clone() const = 0;
#endif
    virtual void ice_throw() const = 0;
};

class ICE_API UserException : public IceUtil::Exception
{
public:

    
    virtual std::string ice_id() const = 0;
#ifndef ICE_CPP11_MAPPING
    ICE_DEPRECATED_API("ice_name() is deprecated, use ice_id() instead.")
    virtual std::string ice_name() const = 0;
    virtual UserException* ice_clone() const = 0;
#endif
    virtual void ice_throw() const = 0;

    virtual void __write(::Ice::OutputStream*) const;
    virtual void __read(::Ice::InputStream*);

    virtual bool __usesClasses() const;

protected:

    virtual void __writeImpl(::Ice::OutputStream*) const = 0;
    virtual void __readImpl(::Ice::InputStream*) = 0;
};

typedef ::IceInternal::Handle<UserException> UserExceptionPtr;

class ICE_API SystemException : public IceUtil::Exception
{
public:

    SystemException(const char*, int);
    virtual ~SystemException() ICE_NOEXCEPT;
    virtual std::string ice_id() const = 0;
#ifndef ICE_CPP11_MAPPING
    ICE_DEPRECATED_API("ice_name() is deprecated.")
    virtual std::string ice_name() const = 0;
    virtual SystemException* ice_clone() const = 0;
#endif
    virtual void ice_throw() const = 0;
};

typedef ::IceInternal::Handle<SystemException> SystemExceptionPtr;

#if defined(__SUNPRO_CC)
//
// COMPILERFIX: With Sun CC the presence of the overloaded operator
// in ProxyHandle.h
//
//   template<class OStream, class Y>
//   OStream& operator<<(OStream& os, ::IceInternal::ProxyHandle<Y> p)
//
// prevents the compiler from using the overloaded operator for
// Exception in IceUtil/Exception.h
//
//   std::ostream& operator<<(std::ostream&, const Exception&);
//
// thus causing a compile error and making these overloads necessary.
//
ICE_API std::ostream& operator<<(std::ostream&, const LocalException&);
ICE_API std::ostream& operator<<(std::ostream&, const UserException&);
ICE_API std::ostream& operator<<(std::ostream&, const SystemException&);
#endif

}

#endif
