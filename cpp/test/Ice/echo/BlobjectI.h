// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Object.h>

class BlobjectI : public Ice::BlobjectAsync
{
public:

    BlobjectI();

    void startBatch();
    void flushBatch();

#ifdef ICE_CPP11_MAPPING

    virtual void ice_invokeAsync(std::vector<Ice::Byte>,
                                 std::function<void(bool, std::vector<Ice::Byte>)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Ice::Current&) override;

#else
    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
                                  const Ice::Current&);
#endif

private:

    bool _startBatch;
    Ice::ObjectPrxPtr _batchProxy;
};

ICE_DEFINE_PTR(BlobjectIPtr, BlobjectI);

#endif
