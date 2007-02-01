// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <IceUtil/Monitor.h>
#include <IceUtil/RecMutex.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/Current.h>

namespace IceInternal
{

class BasicStream;

//
// We need virtual inheritance from shared, because the user might use
// multiple inheritance from IceUtil::Shared.
//
class ICE_API OutgoingAsync : virtual public IceUtil::Shared
{
public:

    OutgoingAsync();
    virtual ~OutgoingAsync();

    virtual void ice_exception(const Ice::Exception&) = 0;

    void __finished(BasicStream&);
    void __finished(const Ice::LocalException&);

protected:

    void __prepare(const Ice::ObjectPrx&, const std::string&, Ice::OperationMode, const Ice::Context*);
    void __send();

    virtual void __response(bool) = 0;

    BasicStream* __is;
    BasicStream* __os;

private:

    void warning(const Ice::Exception&) const;
    void warning(const std::exception&) const;
    void warning() const;

    void cleanup();

    ::Ice::ObjectPrx _proxy;
    ::IceInternal::Handle< ::IceDelegate::Ice::Object> _delegate;
    int _cnt;
    Ice::OperationMode _mode;

    IceUtil::Monitor<IceUtil::RecMutex> _monitor;
};

}

namespace Ice
{

class ICE_API AMI_Object_ice_invoke : public IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const std::vector<Ice::Byte>&) = 0;
    virtual void ice_exception(const Ice::Exception&) = 0;

    void __invoke(const Ice::ObjectPrx&, const std::string& operation, OperationMode,
                  const std::vector<Ice::Byte>&, const Context*);

protected:

    virtual void __response(bool);
};

class ICE_API AMI_Array_Object_ice_invoke : public IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const std::pair<const Byte*, const Byte*>&) = 0;
    virtual void ice_exception(const Ice::Exception&) = 0;

    void __invoke(const Ice::ObjectPrx&, const std::string& operation, OperationMode,
                  const std::pair<const Byte*, const Byte*>&, const Context*);

protected:

    virtual void __response(bool);
};

}

#endif
