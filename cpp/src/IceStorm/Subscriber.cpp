// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>

using namespace IceStorm;
using namespace std;

Subscriber::Subscriber(const TraceLevelsPtr& traceLevels, const Ice::Identity& id) :
    _traceLevels(traceLevels),
    _state(StateActive),
    _id(id)
{
}

Subscriber::~Subscriber()
{
}

bool
Subscriber::inactive() const
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    return _state != StateActive;;
}

bool
Subscriber::error() const
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    return _state == StateError;
}


Ice::Identity
Subscriber::id() const
{
    return _id;
}
