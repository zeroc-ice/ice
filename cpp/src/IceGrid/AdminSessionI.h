//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_ADMINSESSIONI_H
#define ICEGRID_ADMINSESSIONI_H

#include <IceGrid/SessionI.h>
#include <IceGrid/Topics.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{
    class RegistryI;
    class FileIteratorI;

    class AdminSessionI : public BaseSessionI, public AdminSession
    {
    public:
        AdminSessionI(
            const std::string&,
            const std::shared_ptr<Database>&,
            std::chrono::seconds,
            const std::shared_ptr<RegistryI>&);

        Ice::ObjectPrxPtr
        _register(const std::shared_ptr<SessionServantManager>&, const std::shared_ptr<Ice::Connection>&);

        void keepAlive(const Ice::Current& current) override { BaseSessionI::keepAlive(current); }

        AdminPrxPtr getAdmin(const Ice::Current&) const override;
        Ice::ObjectPrxPtr getAdminCallbackTemplate(const Ice::Current&) const override;

        void setObservers(
            RegistryObserverPrxPtr,
            NodeObserverPrxPtr,
            ApplicationObserverPrxPtr,
            AdapterObserverPrxPtr,
            ObjectObserverPrxPtr,
            const Ice::Current&) override;

        void setObserversByIdentity(
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            Ice::Identity,
            const Ice::Current&) override;

        int startUpdate(const Ice::Current&) override;
        void finishUpdate(const Ice::Current&) override;

        std::string getReplicaName(const Ice::Current&) const override;

        FileIteratorPrxPtr openServerLog(std::string, std::string, int, const Ice::Current&) override;
        FileIteratorPrxPtr openServerStdOut(std::string, int, const Ice::Current&) override;
        FileIteratorPrxPtr openServerStdErr(std::string, int, const Ice::Current&) override;

        FileIteratorPrxPtr openNodeStdOut(std::string, int, const Ice::Current&) override;
        FileIteratorPrxPtr openNodeStdErr(std::string, int, const Ice::Current&) override;

        FileIteratorPrxPtr openRegistryStdOut(std::string, int, const Ice::Current&) override;
        FileIteratorPrxPtr openRegistryStdErr(std::string, int, const Ice::Current&) override;

        void destroy(const Ice::Current&) override;

        void removeFileIterator(const Ice::Identity&, const Ice::Current&);

    private:
        void setupObserverSubscription(TopicName, const Ice::ObjectPrxPtr&, bool = false);
        Ice::ObjectPrxPtr addForwarder(const Ice::Identity&, const Ice::Current&);
        Ice::ObjectPrxPtr addForwarder(const Ice::ObjectPrxPtr&);
        FileIteratorPrxPtr addFileIterator(const FileReaderPrxPtr&, const std::string&, int, const Ice::Current&);

        void destroyImpl(bool) override;

        const std::chrono::seconds _timeout;
        const std::string _replicaName;
        AdminPrxPtr _admin;
        std::map<TopicName, std::pair<Ice::ObjectPrxPtr, bool>> _observers;
        std::shared_ptr<RegistryI> _registry;
        Ice::ObjectPrxPtr _adminCallbackTemplate;
    };

    class AdminSessionFactory
    {
    public:
        AdminSessionFactory(
            const std::shared_ptr<SessionServantManager>&,
            const std::shared_ptr<Database>&,
            const std::shared_ptr<ReapThread>&,
            const std::shared_ptr<RegistryI>&);

        Glacier2::SessionPrxPtr createGlacier2Session(const std::string&, const Glacier2::SessionControlPrxPtr&);
        std::shared_ptr<AdminSessionI> createSessionServant(const std::string&);

        const std::shared_ptr<TraceLevels>& getTraceLevels() const;

    private:
        const std::shared_ptr<SessionServantManager> _servantManager;
        const std::shared_ptr<Database> _database;
        const std::chrono::seconds _timeout;
        const std::shared_ptr<ReapThread> _reaper;
        const std::shared_ptr<RegistryI> _registry;
        const bool _filters;
    };

    class AdminSessionManagerI : public Glacier2::SessionManager
    {
    public:
        AdminSessionManagerI(const std::shared_ptr<AdminSessionFactory>&);

        Glacier2::SessionPrxPtr create(std::string, Glacier2::SessionControlPrxPtr, const Ice::Current&) override;

    private:
        const std::shared_ptr<AdminSessionFactory> _factory;
    };

    class AdminSSLSessionManagerI : public Glacier2::SSLSessionManager
    {
    public:
        AdminSSLSessionManagerI(const std::shared_ptr<AdminSessionFactory>&);

        Glacier2::SessionPrxPtr create(Glacier2::SSLInfo, Glacier2::SessionControlPrxPtr, const Ice::Current&) override;

    private:
        const std::shared_ptr<AdminSessionFactory> _factory;
    };

    class FileIteratorI : public FileIterator
    {
    public:
        FileIteratorI(
            const std::shared_ptr<AdminSessionI>&,
            const FileReaderPrxPtr&,
            const std::string&,
            std::int64_t,
            int);

        virtual bool read(int, Ice::StringSeq&, const Ice::Current&);
        virtual void destroy(const Ice::Current&);

    private:
        const std::shared_ptr<AdminSessionI> _session;
        const FileReaderPrxPtr _reader;
        const std::string _filename;
        std::int64_t _offset;
        const int _messageSizeMax;
    };

};

#endif
