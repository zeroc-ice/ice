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

    class AdminSessionI final : public BaseSessionI, public AdminSession
    {
    public:
        AdminSessionI(const std::string&, const std::shared_ptr<Database>&, const std::shared_ptr<RegistryI>&);

        Ice::ObjectPrx _register(const std::shared_ptr<SessionServantManager>&, const Ice::ConnectionPtr&);

        void keepAlive(const Ice::Current&) final {} // no-op

        [[nodiscard]] std::optional<AdminPrx> getAdmin(const Ice::Current&) const final;
        [[nodiscard]] std::optional<Ice::ObjectPrx> getAdminCallbackTemplate(const Ice::Current&) const final;

        void setObservers(
            std::optional<RegistryObserverPrx>,
            std::optional<NodeObserverPrx>,
            std::optional<ApplicationObserverPrx>,
            std::optional<AdapterObserverPrx>,
            std::optional<ObjectObserverPrx>,
            const Ice::Current&) final;

        void setObserversByIdentity(
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            const Ice::Current&) final;

        int startUpdate(const Ice::Current&) final;
        void finishUpdate(const Ice::Current&) final;

        [[nodiscard]] std::string getReplicaName(const Ice::Current&) const final;

        std::optional<FileIteratorPrx> openServerLog(std::string, std::string, int, const Ice::Current&) final;
        std::optional<FileIteratorPrx> openServerStdOut(std::string, int, const Ice::Current&) final;
        std::optional<FileIteratorPrx> openServerStdErr(std::string, int, const Ice::Current&) final;

        std::optional<FileIteratorPrx> openNodeStdOut(std::string, int, const Ice::Current&) final;
        std::optional<FileIteratorPrx> openNodeStdErr(std::string, int, const Ice::Current&) final;

        std::optional<FileIteratorPrx> openRegistryStdOut(std::string, int, const Ice::Current&) final;
        std::optional<FileIteratorPrx> openRegistryStdErr(std::string, int, const Ice::Current&) final;

        void destroy(const Ice::Current&) final;

        void removeFileIterator(const Ice::Identity&, const Ice::Current&);

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
