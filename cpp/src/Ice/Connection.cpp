//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Connection.h"

using namespace Ice;
using namespace std;

// Implement virtual destructors out of line to avoid weak vtables.
Ice::ConnectionInfo::~ConnectionInfo() {}
Ice::TCPConnectionInfo::~TCPConnectionInfo() {}
Ice::UDPConnectionInfo::~UDPConnectionInfo() {}
Ice::WSConnectionInfo::~WSConnectionInfo() {}
Ice::IAPConnectionInfo::~IAPConnectionInfo() {}

Ice::Connection::~Connection() {}

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

future<void>
Ice::Connection::close()
{
    auto sharedPromise = make_shared<promise<void>>();
    close(
        [sharedPromise]() { sharedPromise->set_value(); },
        [sharedPromise](exception_ptr closeException) { sharedPromise->set_exception(closeException); });

    return sharedPromise->get_future();
}
