// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRANSPORT_INFO_I_H
#define ICE_TRANSPORT_INFO_I_H

#include <IceUtil/Mutex.h>
#include <Ice/ConnectionF.h>
#include <Ice/TransportInfo.h>

namespace Ice
{

class ICE_PROTOCOL_API TransportInfoI : public TransportInfo
{
public:

    virtual void flushBatchRequests();

private:

    // This method is for IceInternal::Connection only.
    friend class IceInternal::Connection;
    void setConnection(const IceInternal::ConnectionPtr&);

    IceInternal::ConnectionPtr _connection;
    IceUtil::Mutex _connectionMutex;
};

}

#endif
