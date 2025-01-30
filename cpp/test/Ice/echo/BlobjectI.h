// Copyright (c) ZeroC, Inc.

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include "Ice/Ice.h"

class BlobjectI final : public Ice::BlobjectAsync
{
public:
    BlobjectI();

    void startBatch();
    void flushBatch();
    void setConnection(const Ice::ConnectionPtr&);

    void ice_invokeAsync(
        std::vector<std::byte>,
        std::function<void(bool, const std::vector<std::byte>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

private:
    Ice::ConnectionPtr getConnection(const Ice::Current&);

    bool _startBatch{false};
    std::optional<Ice::ObjectPrx> _batchProxy;
    Ice::ConnectionPtr _connection;
    std::mutex _mutex;
    std::condition_variable _condition;
};

using BlobjectIPtr = std::shared_ptr<BlobjectI>;

#endif
