// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TransportInfoI.h>
#include <Ice/Connection.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::TransportInfoI::flushBatchRequests()
{
    ConnectionPtr connection;

    {
	IceUtil::Mutex::Lock sync(_connectionMutex);
	connection = _connection;
    }

    //
    // We flush outside the mutex lock, to avoid potential deadlocks.
    //
    if(connection)
    {
	connection->flushBatchRequest();
    }
}

void
Ice::TransportInfoI::setConnection(const ConnectionPtr& connection)
{
    IceUtil::Mutex::Lock sync(_connectionMutex);
    _connection = connection;
}

ConnectionPtr
Ice::TransportInfoI::getConnection() const
{
    IceUtil::Mutex::Lock sync(_connectionMutex);
    return _connection;
}
