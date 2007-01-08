// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <Ice/Config.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class BasicStream;

}

namespace Ice
{

typedef IceUtil::Exception Exception;

class ICE_API LocalException : public IceUtil::Exception
{
public:    

    LocalException(const char*, int);
    virtual ~LocalException() throw();
    virtual std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;
};


class ICE_API UserException : public IceUtil::Exception
{
public:    

    virtual std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(::IceInternal::BasicStream*) const = 0;
    virtual void __read(::IceInternal::BasicStream*, bool) = 0;

    virtual bool __usesClasses() const;
};

typedef ::IceInternal::Handle<UserException> UserExceptionPtr;

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
#endif

}

#endif
