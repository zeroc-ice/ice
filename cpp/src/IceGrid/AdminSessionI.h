// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_ADMINSESSIONI_H
#define ICEGRID_ADMINSESSIONI_H

#include "Internal.h"
#include "ReapThread.h"
#include "SessionI.h"
#include "Topics.h"

namespace IceGrid
{
    class RegistryI;
    class FileIteratorI;

    class AdminSessionI final : public BaseSessionI, public AsyncAdminSession
    {
    public:
        AdminSessionI(const std::string&, const std::shared_ptr<Database>&, const std::shared_ptr<RegistryI>&);

        Ice::ObjectPrx _register(const std::shared_ptr<SessionServantManager>&, const Ice::ConnectionPtr&);

        void keepAliveAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final
        {
            response(); // no-op
        }

        void getAdminAsync(
            std::function<void(const std::optional<AdminPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void getAdminCallbackTemplateAsync(
            std::function<void(const std::optional<Ice::ObjectPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void setObserversAsync(
            std::optional<RegistryObserverPrx>,
            std::optional<NodeObserverPrx>,
            std::optional<ApplicationObserverPrx>,
            std::optional<AdapterObserverPrx>,
            std::optional<ObjectObserverPrx>,
            std::function<void()> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void setObserversByIdentityAsync(
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            std::function<void()> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void startUpdateAsync(
            std::function<void(std::int32_t)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void finishUpdateAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void getReplicaNameAsync(
            std::function<void(std::string_view)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void openServerLogAsync(
            std::string,
            std::string,
            std::int32_t,
            std::function<void(const std::optional<FileIteratorPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void openServerStdOutAsync(
            std::string,
            std::int32_t,
            std::function<void(const std::optional<FileIteratorPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void openServerStdErrAsync(
            std::string,
            std::int32_t,
            std::function<void(const std::optional<FileIteratorPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void openNodeStdOutAsync(
            std::string,
            std::int32_t,
            std::function<void(const std::optional<FileIteratorPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void openNodeStdErrAsync(
            std::string,
            std::int32_t,
            std::function<void(const std::optional<FileIteratorPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void openRegistryStdOutAsync(
            std::string,
            std::int32_t,
            std::function<void(const std::optional<FileIteratorPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void openRegistryStdErrAsync(
            std::string,
            std::int32_t,
            std::function<void(const std::optional<FileIteratorPrx>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void destroyAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void removeFileIterator(const Ice::Identity&, const Ice::Current&);

        void destroy();

    private:
        void setupObserverSubscription(TopicName, const std::optional<Ice::ObjectPrx>&, bool = false);
        std::optional<Ice::ObjectPrx> addForwarder(const Ice::Identity&, const Ice::Current&);
        Ice::ObjectPrx addForwarder(Ice::ObjectPrx);
        FileIteratorPrx addFileIterator(FileReaderPrx, const std::string&, int, const Ice::Current&);

        void destroyImpl(bool) final;

        const std::string _replicaName;
        std::optional<AdminPrx> _admin;
        std::map<TopicName, std::pair<Ice::ObjectPrx, bool>> _observers;
        std::shared_ptr<RegistryI> _registry;
        std::optional<Ice::ObjectPrx> _adminCallbackTemplate;
    };

    class AdminSessionFactory
    {
    public:
        AdminSessionFactory(
            const std::shared_ptr<SessionServantManager>&,
            const std::shared_ptr<Database>&,
            const std::shared_ptr<ReapThread>&,
            const std::shared_ptr<RegistryI>&);

        Glacier2::SessionPrx createGlacier2Session(
            const std::string& sessionId,
            const std::optional<Glacier2::SessionControlPrx>& ctl,
            const Ice::ConnectionPtr& con);

        std::shared_ptr<AdminSessionI> createSessionServant(const std::string&);

        [[nodiscard]] const std::shared_ptr<TraceLevels>& getTraceLevels() const;

    private:
        const std::shared_ptr<SessionServantManager> _servantManager;
        const std::shared_ptr<Database> _database;
        const std::shared_ptr<ReapThread> _reaper;
        const std::shared_ptr<RegistryI> _registry;
        const bool _filters{false};
    };

    class AdminSessionManagerI final : public Glacier2::SessionManager
    {
    public:
        AdminSessionManagerI(const std::shared_ptr<AdminSessionFactory>&);

        std::optional<Glacier2::SessionPrx>
        create(std::string, std::optional<Glacier2::SessionControlPrx>, const Ice::Current&) final;

    private:
        const std::shared_ptr<AdminSessionFactory> _factory;
    };

    class AdminSSLSessionManagerI final : public Glacier2::SSLSessionManager
    {
    public:
        AdminSSLSessionManagerI(const std::shared_ptr<AdminSessionFactory>&);

        std::optional<Glacier2::SessionPrx>
        create(Glacier2::SSLInfo, std::optional<Glacier2::SessionControlPrx>, const Ice::Current&) final;

    private:
        const std::shared_ptr<AdminSessionFactory> _factory;
    };

    class FileIteratorI final : public FileIterator
    {
    public:
        FileIteratorI(std::shared_ptr<AdminSessionI>, FileReaderPrx, std::string, std::int64_t, int);

        bool read(int, Ice::StringSeq&, const Ice::Current&) final;
        void destroy(const Ice::Current&) final;

    private:
        const std::shared_ptr<AdminSessionI> _session;
        const FileReaderPrx _reader;
        const std::string _filename;
        std::int64_t _offset;
        const int _messageSizeMax;
    };

};

#endif
