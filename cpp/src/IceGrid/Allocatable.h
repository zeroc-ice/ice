// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_GRID_ALLOCATABLE_H
#define ICE_GRID_ALLOCATABLE_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Time.h>
#include <IceUtil/Timer.h>

#include <IceGrid/Session.h>

#include <list>
#include <set>

namespace IceGrid
{

class SessionI;
typedef IceUtil::Handle<SessionI> SessionIPtr;

class Allocatable;
typedef IceUtil::Handle<Allocatable> AllocatablePtr;

class AllocationRequest : public IceUtil::Mutex, public IceUtil::TimerTask
{
public:

    virtual ~AllocationRequest();

    virtual void allocated(const AllocatablePtr&, const SessionIPtr&) = 0;
    virtual void canceled(const Ice::UserException&) = 0;

    bool pending();
    bool allocate(const AllocatablePtr&, const SessionIPtr&);
    void cancel(const Ice::UserException&);
    void runTimerTask(); // Implementation of IceUtil::TimerTask::runTimerTask()

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

class Allocatable : public virtual IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Allocatable(bool, const AllocatablePtr&);
    virtual ~Allocatable();

    virtual void checkAllocatable();
    virtual bool allocate(const AllocationRequestPtr&, bool = false);
    virtual bool tryAllocate(const AllocationRequestPtr&, bool = false);
    virtual void release(const SessionIPtr&, bool = false);

    bool isAllocatable() const { return _allocatable; }
    SessionIPtr getSession() const;

    virtual bool isEnabled() const = 0;
    virtual void allocated(const SessionIPtr&) = 0;
    virtual void released(const SessionIPtr&) = 0;
    virtual bool canTryAllocate() { return false; }

    virtual void allocatedNoSync(const SessionIPtr&) { ; }
    virtual void releasedNoSync(const SessionIPtr&) { ; }

    bool operator<(const Allocatable&) const;

protected:

    bool allocate(const AllocationRequestPtr&, bool, bool);
    void queueAllocationAttemptFromChild(const AllocatablePtr&);
    bool allocateFromChild(const AllocationRequestPtr&, const AllocatablePtr&, bool, bool);

    void queueAllocationAttempt(const AllocatablePtr&, const AllocationRequestPtr&, bool);
    AllocatablePtr dequeueAllocationAttempt(AllocationRequestPtr&);

    bool _allocatable;
    const AllocatablePtr _parent;

    std::list<std::pair<AllocatablePtr, AllocationRequestPtr> > _requests;
    SessionIPtr _session;
    int _count;
    bool _releasing;
};

};

#endif
