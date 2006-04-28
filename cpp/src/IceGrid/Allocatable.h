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
#include <IceUtil/RecMutex.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Time.h>

#include <IceGrid/WaitQueue.h>
#include <IceGrid/Session.h>

#include <list>
#include <set>

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
    
    virtual void allocated(const AllocatablePtr&, const SessionIPtr&) = 0;
    virtual void canceled(const AllocationException&) = 0;
    
    virtual bool allocateOnce() { return false; }

    bool pending();
    bool finish(const AllocatablePtr&, const SessionIPtr&);
    void cancel(const AllocationException&);
    void expired(bool);

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
};
typedef IceUtil::Handle<AllocationRequest> AllocationRequestPtr;

class ParentAllocationRequest : public AllocationRequest
{
public:

    ParentAllocationRequest(const AllocationRequestPtr&, const AllocatablePtr&);
    
    virtual void allocated(const AllocatablePtr&, const SessionIPtr&);
    virtual void canceled(const AllocationException&);

private:
    
    const AllocationRequestPtr _request;
    const AllocatablePtr _allocatable;
};

class Allocatable : public IceUtil::Shared
{
public:

    Allocatable();
    virtual ~Allocatable();

    virtual void allocate(const AllocationRequestPtr&, bool = true);
    virtual bool tryAllocate(const AllocationRequestPtr&);
    virtual bool release(const SessionIPtr&);

    bool allocatable() const { return _allocatable; }
    bool isAllocated() const;
    SessionIPtr getSession() const;

    virtual void allocated(const SessionIPtr&) {  }
    virtual void released(const SessionIPtr&) {  }

    bool operator<(const Allocatable&) const;

protected:

    bool tryAllocateWithSession(const SessionIPtr&, const AllocatablePtr&);
    bool release(const SessionIPtr&, bool, std::set<AllocatablePtr>&);

    bool _allocatable;
    AllocatablePtr _parent;
    
    IceUtil::RecMutex _allocateMutex;
    std::list<AllocationRequestPtr> _requests;
    SessionIPtr _session;
    int _count;
    std::set<AllocatablePtr> _attempts;
};

};

#endif
