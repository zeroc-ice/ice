// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ALLOCATABLE_H
#define ICE_GRID_ALLOCATABLE_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Time.h>
#include <list>

namespace IceGrid
{

class SessionI;
typedef IceUtil::Handle<SessionI> SessionIPtr;

class Allocatable;
typedef IceUtil::Handle<Allocatable> AllocatablePtr;

class AllocationRequest : public IceUtil::Mutex, public IceUtil::Shared
{
public:

    virtual ~AllocationRequest();
    
    virtual void timeout() = 0;
    virtual void allocated(const AllocatablePtr&) = 0;
    virtual void canceled() = 0;

    bool setAllocatable(const AllocatablePtr&);
    bool checkTimeout(const IceUtil::Time&);
    void cancel();
    void allocate();
    void release(const SessionIPtr&);

    int getTimeout() const { return _timeout; }
    const SessionIPtr& getSession() const { return _session; }

    bool operator<(const AllocationRequest&) const;

protected:

    AllocationRequest(const SessionIPtr&);

private:

    const SessionIPtr _session;
    const int _timeout;
    const IceUtil::Time _expiration;
    bool _canceled;
    AllocatablePtr _allocatable;
};
typedef IceUtil::Handle<AllocationRequest> AllocationRequestPtr;

class Allocatable : public IceUtil::Shared
{
public:

    Allocatable(bool);
    virtual ~Allocatable();

    void allocate(const AllocationRequestPtr&, bool);
    bool tryAllocate(const AllocationRequestPtr&);
    void release(const SessionIPtr&);

    bool allocatable() const { return _allocatable; }

protected:

    bool _allocatable;
    IceUtil::Mutex _allocateMutex;
    std::list<AllocationRequestPtr> _requests;
    AllocationRequestPtr _allocated;
};

};

#endif
