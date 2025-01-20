// Copyright (c) ZeroC, Inc.

#ifndef ICE_COMMUNICATOR_FLUSH_BATCH_ASYNC_H
#define ICE_COMMUNICATOR_FLUSH_BATCH_ASYNC_H

#include "Ice/Communicator.h"
#include "Ice/OutgoingAsync.h"

namespace IceInternal
{
    //
    // Class for handling Ice::Communicator::flushBatchRequests
    // This class needs to be in a public header as it's used by several other classes.
    //
    class CommunicatorFlushBatchAsync : public OutgoingAsyncBase
    {
    public:
        ~CommunicatorFlushBatchAsync() override;

        CommunicatorFlushBatchAsync(const InstancePtr&);

        void flushConnection(const Ice::ConnectionIPtr&, Ice::CompressBatch);
        void invoke(std::string_view, Ice::CompressBatch);

        std::shared_ptr<CommunicatorFlushBatchAsync> shared_from_this()
        {
            return std::static_pointer_cast<CommunicatorFlushBatchAsync>(OutgoingAsyncBase::shared_from_this());
        }

    private:
        void check(bool);

        int _useCount;
    };

    using CommunicatorFlushBatchAsyncPtr = std::shared_ptr<CommunicatorFlushBatchAsync>;
}

#endif
