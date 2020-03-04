//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_SESSIONI_H
#define ICEGRID_SESSIONI_H

#include <IceUtil/Timer.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Session.h>
#include <IceGrid/SessionServantManager.h>
#include <set>

namespace IceGrid
{

class Allocatable;
class AllocationRequest;
class Database;
class SessionI;
class TraceLevels;

class BaseSessionI : public virtual Ice::Object, public std::enable_shared_from_this<BaseSessionI>
{
public:

    virtual ~BaseSessionI() = default;

    virtual void keepAlive(const Ice::Current&);

    std::chrono::steady_clock::time_point timestamp() const;
    void shutdown();
    std::shared_ptr<Glacier2::IdentitySetPrx> getGlacier2IdentitySet();
    std::shared_ptr<Glacier2::StringSetPrx> getGlacier2AdapterIdSet();

    const std::string& getId() const { return _id; }
    virtual void destroyImpl(bool);

protected:

    BaseSessionI(const std::string&, const std::string&, const std::shared_ptr<Database>&);

    const std::string _id;
    const std::string _prefix;
    const std::shared_ptr<TraceLevels> _traceLevels;
    const std::shared_ptr<Database> _database;
    std::shared_ptr<SessionServantManager> _servantManager;
    bool _destroyed;
    std::chrono::steady_clock::time_point _timestamp;

    mutable std::mutex _mutex;
};

struct SessionDestroyedException
{
};

class SessionI final : public BaseSessionI, public Session
{
public:

    SessionI(const std::string&, const std::shared_ptr<Database>&, const IceUtil::TimerPtr&);

    std::shared_ptr<Ice::ObjectPrx> _register(const std::shared_ptr<SessionServantManager>&,
                                              const std::shared_ptr<Ice::Connection>&);

    void keepAlive(const Ice::Current& current) override { BaseSessionI::keepAlive(current); }
    void allocateObjectByIdAsync(Ice::Identity id,
                                 std::function<void(const std::shared_ptr<Ice::ObjectPrx>& returnValue)> response,
                                 std::function<void(std::exception_ptr)> exception, const Ice::Current& current)
                                 override;
    void allocateObjectByTypeAsync(std::string,
                                   std::function<void(const std::shared_ptr<Ice::ObjectPrx>& returnValue)> response,
                                   std::function<void(std::exception_ptr)> exception, const Ice::Current& current)
                                   override;
    void releaseObject(Ice::Identity, const Ice::Current&) override;
    void setAllocationTimeout(int, const Ice::Current&) override;
    void destroy(const Ice::Current&) override;

    int getAllocationTimeout() const;
    const IceUtil::TimerPtr& getTimer() const { return _timer; }

    bool addAllocationRequest(const std::shared_ptr<AllocationRequest>&);
    void removeAllocationRequest(const std::shared_ptr<AllocationRequest>&);
    void addAllocation(const std::shared_ptr<Allocatable>&);
    void removeAllocation(const std::shared_ptr<Allocatable>&);

protected:

    void destroyImpl(bool) override;

    const IceUtil::TimerPtr _timer;
    int _allocationTimeout;
    std::set<std::shared_ptr<AllocationRequest>> _requests;
    std::set<std::shared_ptr<Allocatable>> _allocations;
};

class ClientSessionFactory final
{
public:

    ClientSessionFactory(const std::shared_ptr<SessionServantManager>&, const std::shared_ptr<Database>&,
                         const IceUtil::TimerPtr&,
                         const std::shared_ptr<ReapThread>&);

    std::shared_ptr<Glacier2::SessionPrx> createGlacier2Session(const std::string&,
                                                                const std::shared_ptr<Glacier2::SessionControlPrx>&);
    std::shared_ptr<SessionI> createSessionServant(const std::string&,
                                                   const std::shared_ptr<Glacier2::SessionControlPrx>&);

    const std::shared_ptr<TraceLevels>& getTraceLevels() const;

private:

    const std::shared_ptr<SessionServantManager> _servantManager;
    const std::shared_ptr<Database> _database;
    const IceUtil::TimerPtr _timer;
    const std::shared_ptr<ReapThread> _reaper;
    const bool _filters;
};

class ClientSessionManagerI final : public Glacier2::SessionManager
{
public:

    ClientSessionManagerI(const std::shared_ptr<ClientSessionFactory>&);

    std::shared_ptr<Glacier2::SessionPrx> create(std::string, std::shared_ptr<Glacier2::SessionControlPrx>,
                                                 const Ice::Current&) override;

private:

    const std::shared_ptr<ClientSessionFactory> _factory;
};

class ClientSSLSessionManagerI final : public Glacier2::SSLSessionManager
{
public:

    ClientSSLSessionManagerI(const std::shared_ptr<ClientSessionFactory>&);

    std::shared_ptr<Glacier2::SessionPrx> create(Glacier2::SSLInfo, std::shared_ptr<Glacier2::SessionControlPrx>,
                                                 const Ice::Current&) override;

private:

    const std::shared_ptr<ClientSessionFactory> _factory;
};

};

#endif
