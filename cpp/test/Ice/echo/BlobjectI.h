// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
                                  const Ice::Current&);

private:

    bool _startBatch;
    Ice::ObjectPrx _batchProxy;
};
typedef IceUtil::Handle<BlobjectI> BlobjectIPtr;

#endif
