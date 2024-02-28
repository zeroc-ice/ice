//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Object.h>

class BlobjectI : public Ice::BlobjectAsync
{
public:
    BlobjectI();

    void startBatch();
    void flushBatch();
    void setConnection(const Ice::ConnectionPtr&);

    virtual void ice_invokeAsync(
        std::vector<Ice::Byte>,
        std::function<void(bool, const std::vector<Ice::Byte>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

private:
    Ice::ConnectionPtr getConnection(const Ice::Current&);

    bool _startBatch;
    Ice::ObjectPrxPtr _batchProxy;
    Ice::ConnectionPtr _connection;
    std::mutex _mutex;
    std::condition_variable _condition;
};

using BlobjectIPtr = std::shared_ptr<BlobjectI>;

#endif
