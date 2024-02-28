//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ACM_H
#define ICE_ACM_H

#include <IceUtil/Timer.h>
#include <Ice/ACMF.h>
#include <Ice/Connection.h>
#include <Ice/ConnectionIF.h>
#include <Ice/InstanceF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>

#include <condition_variable>
#include <mutex>
#include <set>
#include <chrono>

namespace IceInternal
{

    class ACMConfig
    {
    public:
        ACMConfig(bool = false);
        ACMConfig(const Ice::PropertiesPtr&, const Ice::LoggerPtr&, const std::string&, const ACMConfig&);

        std::chrono::seconds timeout;
        Ice::ACMHeartbeat heartbeat;
        Ice::ACMClose close;
    };

    class ACMMonitor : public IceUtil::TimerTask
    {
    public:
        virtual void add(const Ice::ConnectionIPtr&) = 0;
        virtual void remove(const Ice::ConnectionIPtr&) = 0;
        virtual void reap(const Ice::ConnectionIPtr&) = 0;

        virtual ACMMonitorPtr
        acm(const std::optional<int>&,
            const std::optional<Ice::ACMClose>&,
            const std::optional<Ice::ACMHeartbeat>&) = 0;
        virtual Ice::ACM getACM() = 0;
    };

    class FactoryACMMonitor : public ACMMonitor, public std::enable_shared_from_this<FactoryACMMonitor>
    {
    public:
        FactoryACMMonitor(const InstancePtr&, const ACMConfig&);
        virtual ~FactoryACMMonitor();

        virtual void add(const Ice::ConnectionIPtr&);
        virtual void remove(const Ice::ConnectionIPtr&);
        virtual void reap(const Ice::ConnectionIPtr&);

        virtual ACMMonitorPtr
        acm(const std::optional<int>&, const std::optional<Ice::ACMClose>&, const std::optional<Ice::ACMHeartbeat>&);
        virtual Ice::ACM getACM();

        void destroy();
        void swapReapedConnections(std::vector<Ice::ConnectionIPtr>&);

    private:
        friend class ConnectionACMMonitor;
        void handleException(const std::exception&);
        void handleException();

        virtual void runTimerTask();

        InstancePtr _instance;
        const ACMConfig _config;

        std::vector<std::pair<Ice::ConnectionIPtr, bool>> _changes;
        std::set<Ice::ConnectionIPtr> _connections;
        std::vector<Ice::ConnectionIPtr> _reapedConnections;
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };

    class ConnectionACMMonitor : public ACMMonitor, public std::enable_shared_from_this<ConnectionACMMonitor>
    {
    public:
        ConnectionACMMonitor(const FactoryACMMonitorPtr&, const IceUtil::TimerPtr&, const ACMConfig&);
        virtual ~ConnectionACMMonitor();

        virtual void add(const Ice::ConnectionIPtr&);
        virtual void remove(const Ice::ConnectionIPtr&);
        virtual void reap(const Ice::ConnectionIPtr&);

        virtual ACMMonitorPtr
        acm(const std::optional<int>&, const std::optional<Ice::ACMClose>&, const std::optional<Ice::ACMHeartbeat>&);
        virtual Ice::ACM getACM();

    private:
        virtual void runTimerTask();

        const FactoryACMMonitorPtr _parent;
        const IceUtil::TimerPtr _timer;
        const ACMConfig _config;

        Ice::ConnectionIPtr _connection;
        std::mutex _mutex;
    };

}

#endif
