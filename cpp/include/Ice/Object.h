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
#include <Ice/StreamF.h>
#include <Ice/Format.h>

namespace IceInternal
{

class Incoming;
class BasicStream;
class Direct;
class GCVisitor;

}

namespace Ice
{

enum DispatchStatus
{
    DispatchOK,
    DispatchUserException,
    DispatchAsync
};


class ICE_API DispatchInterceptorAsyncCallback : public virtual IceUtil::Shared
{
public:
    
    virtual bool response(bool) = 0;
    virtual bool exception(const std::exception&) = 0;
    virtual bool exception() = 0;
};
typedef IceUtil::Handle<DispatchInterceptorAsyncCallback> DispatchInterceptorAsyncCallbackPtr;

class ICE_API Request 
{
public:

    virtual ~Request() {}
    virtual const Current& getCurrent() = 0;
};

class ICE_API Object : virtual public IceUtil::Shared
{
public:

    virtual bool operator==(const Object&) const;
    virtual bool operator<(const Object&) const;

    virtual bool ice_isA(const std::string&, const Current& = Current()) const;
    DispatchStatus ___ice_isA(IceInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&  = Current()) const;
    DispatchStatus ___ice_ping(IceInternal::Incoming&, const Current&);

    virtual std::vector< std::string> ice_ids(const Current& = Current()) const;
    DispatchStatus ___ice_ids(IceInternal::Incoming&, const Current&);

    virtual const std::string& ice_id(const Current& = Current()) const;
    DispatchStatus ___ice_id(IceInternal::Incoming&, const Current&);

    virtual Int ice_operationAttributes(const std::string&) const;

    static const std::string& ice_staticId();

    virtual ObjectPtr ice_clone() const;

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    static std::string __all[];

    virtual DispatchStatus ice_dispatch(Ice::Request&, const DispatchInterceptorAsyncCallbackPtr& = 0);
    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*);

    virtual void __write(const OutputStreamPtr&) const;
    virtual void __read(const InputStreamPtr&);

    virtual bool __gcVisit(IceInternal::GCVisitor&) { return false; };
    virtual void ice_collectable(bool) { };

protected:

    Object() {} // This class is abstract.
    virtual ~Object() {}

    virtual void __writeImpl(IceInternal::BasicStream*) const {}
    virtual void __readImpl(IceInternal::BasicStream*) {}

    virtual void __writeImpl(const OutputStreamPtr&) const;
    virtual void __readImpl(const InputStreamPtr&);

    static void __checkMode(OperationMode, OperationMode);
};

class ICE_API Blobject : virtual public Object
{
public:

    //
    // Returns true if ok, false if user exception.
    //
    virtual bool ice_invoke(const std::vector<Byte>&, std::vector<Byte>&, const Current&) = 0;

    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

class ICE_API BlobjectArray : virtual public Object
{
public:

    //
    // Returns true if ok, false if user exception.
    //
    virtual bool ice_invoke(const std::pair<const Byte*, const Byte*>&, std::vector<Byte>&, const Current&) = 0;

    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

class ICE_API BlobjectAsync : virtual public Object
{
public:

    virtual void ice_invoke_async(const AMD_Object_ice_invokePtr&, const std::vector<Byte>&, const Current&) = 0;

    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

class ICE_API BlobjectArrayAsync : virtual public Object
{
public:

    virtual void ice_invoke_async(const AMD_Object_ice_invokePtr&, const std::pair<const Byte*, const Byte*>&,
                                  const Current&) = 0;

    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

ICE_API void ice_writeObject(const OutputStreamPtr&, const ObjectPtr&);
ICE_API void ice_readObject(const InputStreamPtr&, ObjectPtr&);

}

#endif
