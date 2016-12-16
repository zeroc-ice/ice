// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BATCH_REQUEST_INTERCEPTOR_H
#define ICE_BATCH_REQUEST_INTERCEPTOR_H

#include <IceUtil/Shared.h>
#include <Ice/ProxyF.h>

namespace Ice
{

class BatchRequest
{
public:

    virtual ~BatchRequest()
    {
    }

    virtual void enqueue() const = 0;
    virtual int getSize() const = 0;
    virtual const std::string& getOperation() const = 0;
    virtual const Ice::ObjectPrxPtr& getProxy() const = 0;
};

#ifndef ICE_CPP11_MAPPING

class BatchRequestInterceptor : public IceUtil::Shared
{
public:

    virtual void enqueue(const BatchRequest&, int, int) = 0;
};
typedef IceUtil::Handle<BatchRequestInterceptor> BatchRequestInterceptorPtr;

#endif

}

#endif
