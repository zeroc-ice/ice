// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_SERVER_I_H
#define ICEGRID_SERVER_I_H

#include "../Ice/Timer.h"
#include "Activator.h"
#include "Internal.h"

#include <optional>
#include <set>

#ifndef _WIN32
#    include <sys/types.h> // for uid_t, gid_t
#endif

namespace IceGrid
{
    class NodeI;
    class ServerAdapterI;
    class ServerCommand;
    class DestroyCommand;
    class StopCommand;
    class StartCommand;
    class LoadCommand;

    class ServerI final : public Server, public std::enable_shared_from_this<ServerI>
    {
    public:
        enum InternalServerState
        {
            Loading,
            Inactive,
            Activating,
            WaitForActivation,
            ActivationTimeout,
            Active,
            Deactivating,
            DeactivatingWaitForProcess,
            Destroying,
            Destroyed
        };

        enum ServerActivation
        {
            Always,
            Session,
            OnDemand,
            Manual,
            Disabled
        };

        ServerI(const std::shared_ptr<NodeI>&, std::optional<ServerPrx>, const std::string&, const std::string&, int);

        void waitForApplicationUpdateCompleted();

        void startAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

        void stopAsync(std::function<void()>, std::function<void(std::exception_ptr)>);
        void stopAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;
        void sendSignal(std::string, const Ice::Current&) override;
        void writeMessage(std::string, int, const Ice::Current&) override;

        [[nodiscard]] ServerState getState() const;
        [[nodiscard]] ServerState getState(const Ice::Current&) const override;
        [[nodiscard]] int getPid(const Ice::Current&) const override;

        void setEnabled(bool, const Ice::Current&) override;
        [[nodiscard]] bool isEnabled() const;
        [[nodiscard]] bool isEnabled(const Ice::Current&) const override;
        void setProcessAsync(
            std::optional<Ice::ProcessPrx>,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        [[nodiscard]] std::int64_t getOffsetFromEnd(std::string, int, const Ice::Current&) const override;
        bool read(std::string, std::int64_t, int, std::int64_t&, Ice::StringSeq&, const Ice::Current&) const override;

        [[nodiscard]] bool isAdapterActivatable(const std::string&) const;
        [[nodiscard]] const std::string& getId() const;

        void
            start(ServerActivation, std::function<void()> = nullptr, std::function<void(std::exception_ptr)> = nullptr);
        std::shared_ptr<ServerCommand> load(
            const std::shared_ptr<InternalServerDescriptor>&,
            const std::string&,
            bool,
            std::function<void(ServerPrx, const AdapterPrxDict&, int, int)>,
            std::function<void(std::exception_ptr)>);
        bool checkUpdate(std::shared_ptr<InternalServerDescriptor>, bool, const Ice::Current&) override;
        void checkRemove(bool, const Ice::Current&);
        std::shared_ptr<ServerCommand>
        destroy(const std::string&, int, const std::string&, bool, std::function<void()>);

        void adapterActivated(const std::string&);
        void adapterDeactivated(const std::string&);
        void activationTimedOut();

        void activate();
        void kill();
        void deactivate();
        void update();
        void destroy();
        void terminated(const std::string&, int);
        void shutdown();

        //
        // A proxy to the Process facet of the real Admin object; called by the AdminFacade servant implementation
        //
        [[nodiscard]] std::optional<Ice::ObjectPrx> getProcess() const;

        PropertyDescriptorSeqDict getProperties(const std::shared_ptr<InternalServerDescriptor>&);

        void updateRuntimePropertiesCallback(const std::shared_ptr<InternalServerDescriptor>&);
        void updateRuntimePropertiesCallback(std::exception_ptr, const std::shared_ptr<InternalServerDescriptor>&);

    private:
        void updateImpl(const std::shared_ptr<InternalServerDescriptor>&);
        void checkRevision(const std::string&, const std::string&, int) const;
        void checkNoRestart(const std::shared_ptr<InternalServerDescriptor>&);
        void checkAndUpdateUser(const std::shared_ptr<InternalServerDescriptor>&, bool);
        void updateRevision(const std::string&, int);
        bool checkActivation();
        void checkDestroyed() const;
        void disableOnFailure();

        void setState(InternalServerState, const std::string& = std::string());
        std::shared_ptr<ServerCommand> nextCommand();
        void setStateNoSync(InternalServerState, const std::string& = std::string());

        [[nodiscard]] ServerState toServerState(InternalServerState) const;
        [[nodiscard]] ServerActivation toServerActivation(const std::string&) const;
        [[nodiscard]] ServerDynamicInfo getDynamicInfo() const;
        [[nodiscard]] std::string getFilePath(const std::string&) const;

        const std::shared_ptr<NodeI> _node;
        const std::optional<ServerPrx> _this;
        const std::string _id;
        const std::chrono::seconds _waitTime;
        const std::string _serverDir;
        const std::chrono::seconds _disableOnFailure;

        std::shared_ptr<InternalServerDescriptor> _desc;
#ifndef _WIN32
        uid_t _uid;
        gid_t _gid;
#endif
        InternalServerState _state{ServerI::Inactive};
        ServerActivation _activation{ServerI::Disabled};
        std::chrono::seconds _activationTimeout;
        std::chrono::seconds _deactivationTimeout;
        using ServerAdapterDict = std::map<std::string, std::shared_ptr<ServerAdapterI>>;
        ServerAdapterDict _adapters;
        std::set<std::string> _serverLifetimeAdapters;
        std::optional<Ice::ProcessPrx> _process;
        std::set<std::string> _activatedAdapters;
        std::optional<std::chrono::steady_clock::time_point> _failureTime;
        ServerActivation _previousActivation;
        std::shared_ptr<IceInternal::TimerTask> _timerTask;
        bool _waitForReplication;
        std::string _stdErrFile;
        std::string _stdOutFile;
        Ice::StringSeq _logs;
        PropertyDescriptorSeq _properties;

        std::shared_ptr<DestroyCommand> _destroy;
        std::shared_ptr<StopCommand> _stop;
        std::shared_ptr<LoadCommand> _load;
        std::shared_ptr<StartCommand> _start;

        int _pid{0};

        mutable std::mutex _mutex;
        std::condition_variable _condVar;
    };

    class ServerCommand
    {
    public:
        ServerCommand(const std::shared_ptr<ServerI>&);
        virtual ~ServerCommand() = default;

        virtual bool canExecute(ServerI::InternalServerState) = 0;
        virtual void execute() = 0;
        virtual ServerI::InternalServerState nextState() = 0;

    protected:
        const std::shared_ptr<ServerI> _server;
    };

    class TimedServerCommand : public ServerCommand, public std::enable_shared_from_this<TimedServerCommand>
    {
    public:
        TimedServerCommand(const std::shared_ptr<ServerI>&, IceInternal::TimerPtr, std::chrono::seconds);
        virtual void timeout() = 0;

        void startTimer();
        void stopTimer();

    private:
        IceInternal::TimerPtr _timer;
        std::shared_ptr<IceInternal::TimerTask> _timerTask;
        std::chrono::seconds _timeout;
    };

    class DestroyCommand : public ServerCommand
    {
    public:
        DestroyCommand(const std::shared_ptr<ServerI>&, bool, bool);

        bool canExecute(ServerI::InternalServerState) override;
        ServerI::InternalServerState nextState() override;
        void execute() override;

        void addCallback(std::function<void()>);
        void finished();
        [[nodiscard]] bool loadFailure() const;
        [[nodiscard]] bool clearDir() const;

    private:
        const bool _loadFailure;
        const bool _clearDir;
        std::vector<std::function<void()>> _destroyCB;
    };

    class StopCommand : public TimedServerCommand
    {
    public:
        StopCommand(const std::shared_ptr<ServerI>&, const IceInternal::TimerPtr&, std::chrono::seconds, bool = true);

        static bool isStopped(ServerI::InternalServerState);

        bool canExecute(ServerI::InternalServerState) override;
        ServerI::InternalServerState nextState() override;
        void execute() override;
        void timeout() override;

        void addCallback(std::function<void()>, std::function<void(std::exception_ptr)>);
        void failed(const std::string& reason);
        void finished();

    private:
        std::vector<std::pair<std::function<void()>, std::function<void(std::exception_ptr)>>> _stopCB;
        bool _deactivate;
    };

    class StartCommand : public TimedServerCommand
    {
    public:
        using TimedServerCommand::TimedServerCommand;

        bool canExecute(ServerI::InternalServerState) override;
        ServerI::InternalServerState nextState() override;
        void execute() override;
        void timeout() override;

        void addCallback(std::function<void()>, std::function<void(std::exception_ptr)>);
        void failed(const std::string&);
        void finished();

    private:
        std::vector<std::pair<std::function<void()>, std::function<void(std::exception_ptr)>>> _startCB;
    };

    class LoadCommand : public ServerCommand
    {
    public:
        LoadCommand(
            const std::shared_ptr<ServerI>&,
            const std::shared_ptr<InternalServerDescriptor>&,
            const std::shared_ptr<TraceLevels>&);

        bool canExecute(ServerI::InternalServerState) override;
        ServerI::InternalServerState nextState() override;
        void execute() override;

        void setUpdate(const std::shared_ptr<InternalServerDescriptor>&, bool);
        [[nodiscard]] bool clearDir() const;
        [[nodiscard]] std::shared_ptr<InternalServerDescriptor> getInternalServerDescriptor() const;
        void addCallback(
            std::function<void(ServerPrx, const AdapterPrxDict&, int, int)>,
            std::function<void(std::exception_ptr)>);
        void startRuntimePropertiesUpdate(Ice::ObjectPrx);
        bool finishRuntimePropertiesUpdate(const std::shared_ptr<InternalServerDescriptor>&, Ice::ObjectPrx);
        void failed(std::exception_ptr);
        void finished(const ServerPrx&, const AdapterPrxDict&, std::chrono::seconds, std::chrono::seconds);

    private:
        std::vector<std::pair<
            std::function<void(ServerPrx, const AdapterPrxDict&, int, int)>,
            std::function<void(std::exception_ptr)>>>
            _loadCB;
        bool _clearDir;
        std::shared_ptr<InternalServerDescriptor> _desc;
        std::shared_ptr<InternalServerDescriptor> _runtime;
        bool _updating{false};
        std::shared_ptr<TraceLevels> _traceLevels;
    };
}

#endif
