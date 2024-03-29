//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Connection.h"

using namespace Ice;
using namespace std;

void
Ice::Connection::flushBatchRequests(CompressBatch compress)
{
    flushBatchRequestsAsync(compress).get();
}

std::future<void>
Ice::Connection::flushBatchRequestsAsync(CompressBatch compress)
{
    auto promise = std::make_shared<std::promise<void>>();
    flushBatchRequestsAsync(
        compress,
        [promise](std::exception_ptr ex) { promise->set_exception(ex); },
        [promise](bool) { promise->set_value(); });
    return promise->get_future();
}

void
Ice::Connection::heartbeat()
{
    heartbeatAsync().get();
}

std::future<void>
Ice::Connection::heartbeatAsync()
{
    auto promise = std::make_shared<std::promise<void>>();
    heartbeatAsync(
        [promise](std::exception_ptr ex) { promise->set_exception(ex); },
        [promise](bool) { promise->set_value(); });
    return promise->get_future();
}
