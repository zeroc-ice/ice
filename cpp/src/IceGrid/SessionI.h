// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_SESSIONI_H
#define ICEGRID_SESSIONI_H

#include "../Ice/Timer.h"
#include "IceGrid/Session.h"
#include "ReapThread.h"
#include "SessionServantManager.h"
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
        ~BaseSessionI() override = default;

        // Return value is never used. Just returns nullopt when the session is destroyed.
        [[nodiscard]] std::optional<std::chrono::steady_clock::time_point> timestamp() const noexcept;

        void shutdown();
        std::optional<Glacier2::IdentitySetPrx> getGlacier2IdentitySet();
        std::optional<Glacier2::StringSetPrx> getGlacier2AdapterIdSet();

        [[nodiscard]] const std::string& getId() const { return _id; }
        virtual void destroyImpl(bool);

    protected:
        BaseSessionI(std::string, std::string, const std::shared_ptr<Database>&);

        const std::string _id;
        const std::string _prefix;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::shared_ptr<Database> _database;
        std::shared_ptr<SessionServantManager> _servantManager;
        bool _destroyed{false};

        mutable std::mutex _mutex;
    };

    struct SessionDestroyedException
    {
    };

    class SessionI final : public BaseSessionI, public AsyncSession
    {
    public:
        SessionI(const std::string&, const std::shared_ptr<Database>&, IceInternal::TimerPtr);

        Ice::ObjectPrx _register(const std::shared_ptr<SessionServantManager>&, const Ice::ConnectionPtr&);

        void keepAliveAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final
        {
            response(); // no-op
        }

        void allocateObjectByIdAsync(
            Ice::Identity id,
            std::function<void(const std::optional<Ice::ObjectPrx>& returnValue)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current& current) final;

        void allocateObjectByTypeAsync(
            std::string,
            std::function<void(const std::optional<Ice::ObjectPrx>& returnValue)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current& current) final;

        void releaseObjectAsync(
            Ice::Identity,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void setAllocationTimeoutAsync(
            std::int32_t,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void destroyAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void destroy();

        [[nodiscard]] int getAllocationTimeout() const;
        [[nodiscard]] const IceInternal::TimerPtr& getTimer() const { return _timer; }

        bool addAllocationRequest(const std::shared_ptr<AllocationRequest>&);
        void removeAllocationRequest(const std::shared_ptr<AllocationRequest>&);
        void addAllocation(const std::shared_ptr<Allocatable>&);
        void removeAllocation(const std::shared_ptr<Allocatable>&);

    private:
        void destroyImpl(bool) final;

        const IceInternal::TimerPtr _timer;
        int _allocationTimeout{-1};
        std::set<std::shared_ptr<AllocationRequest>> _requests;
        std::set<std::shared_ptr<Allocatable>> _allocations;
    };

    class ClientSessionFactory final
    {
    public:
        ClientSessionFactory(
            const std::shared_ptr<SessionServantManager>&,
            const std::shared_ptr<Database>&,
            IceInternal::TimerPtr,
            const std::shared_ptr<ReapThread>&);

        Glacier2::SessionPrx createGlacier2Session(
            const std::string& sessionId,
            const std::optional<Glacier2::SessionControlPrx>& ctl,
            const Ice::ConnectionPtr& con);

        std::shared_ptr<SessionI> createSessionServant(const std::string&);

        [[nodiscard]] const std::shared_ptr<TraceLevels>& getTraceLevels() const;

    private:
        const std::shared_ptr<SessionServantManager> _servantManager;
        const std::shared_ptr<Database> _database;
        const IceInternal::TimerPtr _timer;
        const std::shared_ptr<ReapThread> _reaper;
        const bool _filters{false};
    };

    class ClientSessionManagerI final : public Glacier2::SessionManager
    {
    public:
        ClientSessionManagerI(const std::shared_ptr<ClientSessionFactory>&);

        std::optional<Glacier2::SessionPrx>
        create(std::string, std::optional<Glacier2::SessionControlPrx>, const Ice::Current&) final;

    private:
        const std::shared_ptr<ClientSessionFactory> _factory;
    };

    class ClientSSLSessionManagerI final : public Glacier2::SSLSessionManager
    {
    public:
        ClientSSLSessionManagerI(const std::shared_ptr<ClientSessionFactory>&);

        std::optional<Glacier2::SessionPrx>
        create(Glacier2::SSLInfo, std::optional<Glacier2::SessionControlPrx>, const Ice::Current&) final;

    private:
        const std::shared_ptr<ClientSessionFactory> _factory;
    };

};

#endif
