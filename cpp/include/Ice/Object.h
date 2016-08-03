// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/IncomingAsyncF.h>
#include <Ice/Current.h>
#include <Ice/Format.h>

namespace Ice
{

class OutputStream;
class InpputStream;

}

namespace IceInternal
{

class Incoming;
class Direct;
class GCVisitor;

}

namespace Ice
{

ICE_API extern const Current noExplicitCurrent;

enum DispatchStatus
{
    DispatchOK,
    DispatchUserException,
    DispatchAsync
};


class ICE_API DispatchInterceptorAsyncCallback
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual ~DispatchInterceptorAsyncCallback();

    virtual bool response(bool) = 0;
    virtual bool exception(const std::exception&) = 0;
    virtual bool exception() = 0;
};
ICE_DEFINE_PTR(DispatchInterceptorAsyncCallbackPtr, DispatchInterceptorAsyncCallback);

class ICE_API Request
{
public:

    virtual ~Request();
    virtual const Current& getCurrent() = 0;
};

#ifdef ICE_CPP11_MAPPING
class ICE_API Object
{
public:

    virtual ~Object() = default;

    virtual bool ice_isA(std::string, const Current& = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_isA(IceInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&  = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_ping(IceInternal::Incoming&, const Current&);

    virtual std::vector< std::string> ice_ids(const Current& = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_ids(IceInternal::Incoming&, const Current&);

    virtual std::string ice_id(const Current& = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_id(IceInternal::Incoming&, const Current&);

    static const std::string& ice_staticId();

    static std::string __all[];

    virtual DispatchStatus ice_dispatch(Ice::Request&, const DispatchInterceptorAsyncCallbackPtr& = 0);
    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);

    struct Ice_invokeResult
    {
        bool returnValue;
        std::vector<::Ice::Byte> outParams;
    };

protected:

    static void __checkMode(OperationMode, OperationMode);
};
#else
class ICE_API Object  : public virtual IceUtil::Shared
{
public:

    virtual bool operator==(const Object&) const;
    virtual bool operator<(const Object&) const;

    virtual bool ice_isA(const std::string&, const Current& = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_isA(IceInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&  = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_ping(IceInternal::Incoming&, const Current&);

    virtual std::vector< std::string> ice_ids(const Current& = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_ids(IceInternal::Incoming&, const Current&);

    virtual const std::string& ice_id(const Current& = Ice::noExplicitCurrent) const;
    DispatchStatus ___ice_id(IceInternal::Incoming&, const Current&);

    virtual Int ice_operationAttributes(const std::string&) const;

    virtual void __write(Ice::OutputStream*) const;
    virtual void __read(Ice::InputStream*);

    virtual bool __gcVisit(IceInternal::GCVisitor&) { return false; };
    virtual void ice_collectable(bool) { };

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    static const std::string& ice_staticId();

    virtual ObjectPtr ice_clone() const;

    static std::string __all[];

    virtual DispatchStatus ice_dispatch(Ice::Request&, const DispatchInterceptorAsyncCallbackPtr& = 0);
    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);

protected:

    Object() {} // This class is abstract.
    virtual ~Object() {}

protected:

    virtual void __writeImpl(Ice::OutputStream*) const {}
    virtual void __readImpl(Ice::InputStream*) {}

    static void __checkMode(OperationMode, OperationMode);
};
#endif

class ICE_API Blobject : public virtual Object
{
public:

    //
    // Returns true if ok, false if user exception.
    //
    virtual bool ice_invoke(const std::vector<Byte>&, std::vector<Byte>&, const Current&) = 0;

    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

class ICE_API BlobjectArray : public virtual Object
{
public:

    //
    // Returns true if ok, false if user exception.
    //
    virtual bool ice_invoke(const std::pair<const Byte*, const Byte*>&, std::vector<Byte>&, const Current&) = 0;

    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

class ICE_API BlobjectAsync : public virtual Object
{
public:

#ifdef ICE_CPP11_MAPPING
    virtual void ice_invokeAsync(std::vector<Byte>,
                                 std::function<void(bool, std::vector<Byte>)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Current&) = 0;
#else
    virtual void ice_invoke_async(const AMD_Object_ice_invokePtr&, const std::vector<Byte>&, const Current&) = 0;
#endif
    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

class ICE_API BlobjectArrayAsync : public virtual Object
{
public:

#ifdef ICE_CPP11_MAPPING
    virtual void ice_invokeAsync(std::pair<const Byte*, const Byte*>,
                                 std::function<void(bool, std::pair<const Byte*, const Byte*>)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Current&) = 0;
#else
    virtual void ice_invoke_async(const AMD_Object_ice_invokePtr&, const std::pair<const Byte*, const Byte*>&,
                                  const Current&) = 0;
#endif
    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

}

#endif
