// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Ice.h>
#include <Glacier/Missive.h>

namespace Glacier
{

class Blobject : public Ice::Blobject
{
public:
    
    Blobject(const Ice::CommunicatorPtr&);
    virtual ~Blobject();
    
    void destroy();
    virtual MissiveQueuePtr modifyProxy(Ice::ObjectPrx&, const Ice::Current&);

protected:

    Ice::CommunicatorPtr _communicator;
    Ice::LoggerPtr _logger;

private:

    MissiveQueuePtr getMissiveQueue();

    MissiveQueuePtr _missiveQueue;
    IceUtil::ThreadControl _missiveQueueControl;
    IceUtil::Mutex _missiveQueueMutex;
};

}

#endif
