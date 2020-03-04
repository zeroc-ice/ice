//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_ALLOCATABLE_H
#define ICE_GRID_ALLOCATABLE_H

#include <IceUtil/Timer.h>

#include <IceGrid/Session.h>

#include <list>
#include <set>

namespace IceGrid
{

class Allocatable;
class SessionI;

class AllocationRequest : public IceUtil::TimerTask, public std::enable_shared_from_this<AllocationRequest>
{
public:

    virtual ~AllocationRequest() = default;

    virtual void allocated(const std::shared_ptr<Allocatable>&, const std::shared_ptr<SessionI>&) = 0;
    virtual void canceled(std::exception_ptr) = 0;

    bool pending();
    bool allocate(const std::shared_ptr<Allocatable>&, const std::shared_ptr<SessionI>&);
    void cancel(std::exception_ptr);

    void runTimerTask() override;

    int getTimeout() const { return _timeout; }
    const std::shared_ptr<SessionI>& getSession() const { return _session; }
    bool isCanceled() const;

    bool operator<(const AllocationRequest&) const;

protected:

    AllocationRequest(const std::shared_ptr<SessionI>&);

private:

    enum State
    {
        Initial,
        Pending,
        Canceled,
        Allocated
    };

    const std::shared_ptr<SessionI> _session;
    const int _timeout;
    State _state;

    mutable std::mutex _mutex;
};

class Allocatable: public virtual std::enable_shared_from_this<Allocatable>
{
public:

    Allocatable(bool, const std::shared_ptr<Allocatable>&);
    virtual ~Allocatable() = default;

    virtual void checkAllocatable();
    virtual bool allocate(const std::shared_ptr<AllocationRequest>&, bool = false);
    virtual bool tryAllocate(const std::shared_ptr<AllocationRequest>&, bool = false);
    virtual void release(const std::shared_ptr<SessionI>&, bool = false);

    bool isAllocatable() const { return _allocatable; }
    std::shared_ptr<SessionI> getSession() const;

    virtual bool isEnabled() const = 0;
    virtual void allocated(const std::shared_ptr<SessionI>&) = 0;
    virtual void released(const std::shared_ptr<SessionI>&) = 0;
    virtual bool canTryAllocate() { return false; }

    virtual void allocatedNoSync(const std::shared_ptr<SessionI>&) {}
    virtual void releasedNoSync(const std::shared_ptr<SessionI>&) {}

    bool operator<(const Allocatable&) const;

protected:

    bool allocate(const std::shared_ptr<AllocationRequest>&, bool, bool);
    void queueAllocationAttemptFromChild(const std::shared_ptr<Allocatable>&);
    bool allocateFromChild(const std::shared_ptr<AllocationRequest>&, const std::shared_ptr<Allocatable>&, bool, bool);

    void queueAllocationAttempt(const std::shared_ptr<Allocatable>&, const std::shared_ptr<AllocationRequest>&, bool);
    std::shared_ptr<Allocatable> dequeueAllocationAttempt(std::shared_ptr<AllocationRequest>&);

    bool _allocatable;
    const std::shared_ptr<Allocatable> _parent;

    std::list<std::pair<std::shared_ptr<Allocatable>, std::shared_ptr<AllocationRequest>>> _requests;
    std::shared_ptr<SessionI> _session;
    int _count;
    bool _releasing;

    mutable std::mutex _mutex;
    std::condition_variable _condVar;
};

};

#endif
