// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Object.h>

class BlobjectI : public Ice::BlobjectAsync, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    BlobjectI();

    void startBatch();
    void flushBatch();
    void setConnection(const Ice::ConnectionPtr&);

#ifdef ICE_CPP11_MAPPING

    virtual void ice_invokeAsync(std::vector<Ice::Byte>,
                                 std::function<void(bool, const std::vector<Ice::Byte>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Ice::Current&) override;

#else
    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
                                  const Ice::Current&);
#endif

private:

    Ice::ConnectionPtr getConnection(const Ice::Current&);

    bool _startBatch;
    Ice::ObjectPrxPtr _batchProxy;
    Ice::ConnectionPtr _connection;
};

ICE_DEFINE_PTR(BlobjectIPtr, BlobjectI);

#endif
