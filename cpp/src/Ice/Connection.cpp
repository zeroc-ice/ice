//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_API_EXPORTS
#    define ICE_API_EXPORTS
#endif
#include <Ice/Connection.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#    pragma warning(disable : 4458) // declaration of ... hides class member
#elif defined(__clang__)
#    pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

Ice::ConnectionInfo::~ConnectionInfo() {}

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

Ice::IPConnectionInfo::~IPConnectionInfo() {}

Ice::TCPConnectionInfo::~TCPConnectionInfo() {}

Ice::UDPConnectionInfo::~UDPConnectionInfo() {}

Ice::WSConnectionInfo::~WSConnectionInfo() {}
