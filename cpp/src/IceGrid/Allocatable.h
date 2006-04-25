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

#include <IceGrid/WaitQueue.h>

#include <list>


namespace IceGrid
{

class SessionI;
typedef IceUtil::Handle<SessionI> SessionIPtr;

class Allocatable;
typedef IceUtil::Handle<Allocatable> AllocatablePtr;

class AllocationRequest : public IceUtil::Mutex, public WaitItem
{
public:

    virtual ~AllocationRequest();
    
    virtual void timeout() = 0;
    virtual void allocated(const AllocatablePtr&) = 0;
    virtual void canceled() = 0;

    bool setAllocatable(const AllocatablePtr&);
    void cancel();
    virtual void expired(bool);

    void allocate();
    void release();

    int getTimeout() const { return _timeout; }
    const SessionIPtr& getSession() const { return _session; }

    bool operator<(const AllocationRequest&) const;

protected:

    AllocationRequest(const SessionIPtr&);

private:

    enum State
    {
	Initial,
	Pending,
	Canceled,
	Allocated
    };

    const SessionIPtr _session;
    const int _timeout;
    State _state;
    AllocatablePtr _allocatable;
};
typedef IceUtil::Handle<AllocationRequest> AllocationRequestPtr;

class Allocatable : public IceUtil::Shared
{
public:

    Allocatable(bool);
    virtual ~Allocatable();

    virtual void allocate(const AllocationRequestPtr&, bool);
    virtual bool tryAllocate(const AllocationRequestPtr&);
    virtual bool release(const SessionIPtr&);

    bool allocatable() const { return _allocatable; }
    bool isAllocated() const;

    virtual void allocated() {  }
    virtual void released() {  }

protected:

    bool _allocatable;
    IceUtil::Mutex _allocateMutex;
    std::list<AllocationRequestPtr> _requests;
    AllocationRequestPtr _allocated;
};

};

#endif
