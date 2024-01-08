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

    AdminSessionI(const std::string&, const std::shared_ptr<Database>&,
                  std::chrono::seconds, const std::shared_ptr<RegistryI>&);

    std::shared_ptr<Ice::ObjectPrx> _register(const std::shared_ptr<SessionServantManager>&,
                                              const std::shared_ptr<Ice::Connection>&);

    void keepAlive(const Ice::Current& current) override { BaseSessionI::keepAlive(current); }

    std::shared_ptr<AdminPrx> getAdmin(const Ice::Current&) const override;
    std::shared_ptr<Ice::ObjectPrx> getAdminCallbackTemplate(const Ice::Current&) const override;

    void setObservers(std::shared_ptr<RegistryObserverPrx>, std::shared_ptr<NodeObserverPrx>,
                      std::shared_ptr<ApplicationObserverPrx>, std::shared_ptr<AdapterObserverPrx>,
                      std::shared_ptr<ObjectObserverPrx>, const Ice::Current&) override;

    void setObserversByIdentity(Ice::Identity, Ice::Identity, Ice::Identity, Ice::Identity, Ice::Identity,
                                const Ice::Current&) override;

    int startUpdate(const Ice::Current&) override;
    void finishUpdate(const Ice::Current&) override;

    std::string getReplicaName(const Ice::Current&) const override;

    std::shared_ptr<FileIteratorPrx> openServerLog(std::string, std::string, int, const Ice::Current&) override;
    std::shared_ptr<FileIteratorPrx> openServerStdOut(std::string, int, const Ice::Current&) override;
    std::shared_ptr<FileIteratorPrx> openServerStdErr(std::string, int, const Ice::Current&) override;

    std::shared_ptr<FileIteratorPrx> openNodeStdOut(std::string, int, const Ice::Current&) override;
    std::shared_ptr<FileIteratorPrx> openNodeStdErr(std::string, int, const Ice::Current&) override;

    std::shared_ptr<FileIteratorPrx> openRegistryStdOut(std::string, int, const Ice::Current&) override;
    std::shared_ptr<FileIteratorPrx> openRegistryStdErr(std::string, int, const Ice::Current&) override;

    void destroy(const Ice::Current&) override;

    void removeFileIterator(const Ice::Identity&, const Ice::Current&);

private:

    void setupObserverSubscription(TopicName, const std::shared_ptr<Ice::ObjectPrx>&, bool = false);
    std::shared_ptr<Ice::ObjectPrx> addForwarder(const Ice::Identity&, const Ice::Current&);
    std::shared_ptr<Ice::ObjectPrx> addForwarder(const std::shared_ptr<Ice::ObjectPrx>&);
    std::shared_ptr<FileIteratorPrx> addFileIterator(const std::shared_ptr<FileReaderPrx>&, const std::string&, int,
                                                     const Ice::Current&);

    void destroyImpl(bool) override;

    const std::chrono::seconds _timeout;
    const std::string _replicaName;
    std::shared_ptr<AdminPrx> _admin;
    std::map<TopicName, std::pair<std::shared_ptr<Ice::ObjectPrx>, bool>> _observers;
    std::shared_ptr<RegistryI> _registry;
    std::shared_ptr<Ice::ObjectPrx> _adminCallbackTemplate;
};

class AdminSessionFactory
{
public:

    AdminSessionFactory(const std::shared_ptr<SessionServantManager>&, const std::shared_ptr<Database>&,
                        const std::shared_ptr<ReapThread>&, const std::shared_ptr<RegistryI>&);

    std::shared_ptr<Glacier2::SessionPrx> createGlacier2Session(const std::string&,
                                                                const std::shared_ptr<Glacier2::SessionControlPrx>&);
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

    std::shared_ptr<Glacier2::SessionPrx> create(std::string, std::shared_ptr<Glacier2::SessionControlPrx>,
                                                 const Ice::Current&) override;

private:

    const std::shared_ptr<AdminSessionFactory> _factory;
};

class AdminSSLSessionManagerI : public Glacier2::SSLSessionManager
{
public:

    AdminSSLSessionManagerI(const std::shared_ptr<AdminSessionFactory>&);

    std::shared_ptr<Glacier2::SessionPrx> create(Glacier2::SSLInfo, std::shared_ptr<Glacier2::SessionControlPrx>,
                                                 const Ice::Current&) override;

private:

    const std::shared_ptr<AdminSessionFactory> _factory;
};

class FileIteratorI : public FileIterator
{
public:

    FileIteratorI(const std::shared_ptr<AdminSessionI>&, const std::shared_ptr<FileReaderPrx>&, const std::string&,
                  long long, int);

    virtual bool read(int, Ice::StringSeq&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

private:

    const std::shared_ptr<AdminSessionI> _session;
    const std::shared_ptr<FileReaderPrx> _reader;
    const std::string _filename;
    long long _offset;
    const int _messageSizeMax;
};

};

#endif
