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
    
    virtual bool allocated(const AllocatablePtr&, const SessionIPtr&) = 0;
    virtual void canceled(const AllocationException&) = 0;
    
    bool pending();
    bool finish(const AllocatablePtr&, const SessionIPtr&);
    void cancel(const AllocationException&);
    void expired(bool);

    int getTimeout() const { return _timeout; }
    const SessionIPtr& getSession() const { return _session; }
    bool isCanceled() const;

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

class Allocatable : virtual public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Allocatable(bool, const AllocatablePtr&);
    virtual ~Allocatable();

    virtual void checkAllocatable();
    virtual bool allocate(const AllocationRequestPtr&, bool = false);
    virtual bool tryAllocate(const AllocationRequestPtr&, bool = false);
    virtual bool release(const SessionIPtr&, bool = false);

    bool isAllocatable() const { return _allocatable; }
    SessionIPtr getSession() const;

    virtual void allocated(const SessionIPtr&) = 0;
    virtual void released(const SessionIPtr&) = 0;
    virtual bool canTryAllocate() { return false; }

    bool operator<(const Allocatable&) const;

protected:

    bool allocate(const AllocationRequestPtr&, bool, bool);
    bool allocateFromChild(const AllocationRequestPtr&, const AllocatablePtr&, bool, bool);
    
    void queueAllocationAttempt(const AllocatablePtr&, const AllocationRequestPtr&, bool);
    AllocatablePtr dequeueAllocationAttempt(AllocationRequestPtr&);

    const bool _allocatable;
    const AllocatablePtr _parent;
    
    std::list<std::pair<AllocatablePtr, AllocationRequestPtr> > _requests;
    std::set<AllocatablePtr> _attempts;
    SessionIPtr _session;
    int _count;
    bool _releasing;
};

};

#endif
