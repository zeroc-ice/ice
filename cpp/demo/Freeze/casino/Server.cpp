// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <Casino.h>
#include <CasinoStore.h>
#include <BetResolver.h>
#include <BankI.h>
#include <BetI.h>
#include <PlayerI.h>

using namespace std;

class CasinoServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);

    CasinoServer(const string&);

private:

    map<string, string> createTypeMap(const string&);
    const string _envName;

    CasinoStore::PersistentBankPrx _bankPrx;
    Freeze::TransactionalEvictorPtr _bankEvictor;
    Freeze::TransactionalEvictorPtr _playerEvictor;
    Freeze::TransactionalEvictorPtr _betEvictor;
    BetResolver _betResolver;
    int _bankEdge;
};

int
main(int argc, char* argv[])
{
    CasinoServer app("db");
    return app.main(argc, argv, "config.server");
}

template<class T>
class ObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr
    create(const string& /*type*/)
    {
        return new T;
    }

    virtual void
    destroy()
    {
    }
};

int
CasinoServer::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    //
    // Initialize pseudo-random number generator
    //
    srand((unsigned int)IceUtil::Time::now().toMicroSeconds());

    Ice::PropertiesPtr properties = communicator()->getProperties();

    _bankEdge = properties->getPropertyAsInt("Bank.Edge");
    if(_bankEdge < 1)
    {
        _bankEdge = 1;
    }
    cout << "Bank edge is " << _bankEdge << endl;

    //
    // Create an object adapter
    //
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Casino");

    //
    // Register factories
    //
    communicator()->addObjectFactory(new ObjectFactory<BankI>, CasinoStore::PersistentBank::ice_staticId());
    communicator()->addObjectFactory(new ObjectFactory<PlayerI>, CasinoStore::PersistentPlayer::ice_staticId());
    communicator()->addObjectFactory(new ObjectFactory<BetI>, CasinoStore::PersistentBet::ice_staticId());

    //
    // Create evictors; each type gets its own type-specific evictor
    //

    //
    // Bank evictor
    //

    class BankInitializer : public Freeze::ServantInitializer
    {
    public:

        BankInitializer(CasinoServer& server) :
            _server(server)
        {
        }

        virtual void
        initialize(const Ice::ObjectAdapterPtr& /*adapter*/, const Ice::Identity& /*identity*/, const string& /*facet*/,
                   const Ice::ObjectPtr& servant)
        {
            BankI* bank = dynamic_cast<BankI*>(servant.get());
            bank->init(_server._bankPrx, _server._bankEvictor, _server._playerEvictor, _server._betEvictor,
                       _server._betResolver, _server._bankEdge);
        }

    private:
        CasinoServer& _server;
    };

    _bankEvictor =
        Freeze::createTransactionalEvictor(adapter, _envName, "bank",
                                           createTypeMap(CasinoStore::PersistentBank::ice_staticId()),
                                           new BankInitializer(*this));

    int size = properties->getPropertyAsInt("Bank.EvictorSize");
    if(size > 0)
    {
        _bankEvictor->setSize(size);
    }

    adapter->addServantLocator(_bankEvictor, "bank");

    //
    // Player evictor
    //

    class PlayerInitializer : public Freeze::ServantInitializer
    {
    public:

        PlayerInitializer(CasinoServer& server) :
            _server(server)
        {
        }

        virtual void
        initialize(const Ice::ObjectAdapterPtr& adapter, const Ice::Identity& identity, const string& /*facet*/,
                   const Ice::ObjectPtr& servant)
        {
            CasinoStore::PersistentPlayerPrx prx =
                CasinoStore::PersistentPlayerPrx::uncheckedCast(adapter->createProxy(identity));

            PlayerI* player = dynamic_cast<PlayerI*>(servant.get());
            player->init(prx,  _server._playerEvictor, _server._bankPrx);
        }

    private:
        CasinoServer& _server;
    };

    _playerEvictor =
        Freeze::createTransactionalEvictor(adapter, _envName, "player",
                                           createTypeMap(CasinoStore::PersistentPlayer::ice_staticId()),
                                           new PlayerInitializer(*this));

    size = properties->getPropertyAsInt("Player.EvictorSize");
    if(size > 0)
    {
        _playerEvictor->setSize(size);
    }

    adapter->addServantLocator(_playerEvictor, "player");

    //
    // Bet evictor
    //

    class BetInitializer : public Freeze::ServantInitializer
    {
    public:

        BetInitializer(CasinoServer& server) :
            _server(server)
        {
        }

        virtual void
        initialize(const Ice::ObjectAdapterPtr& /*adapter*/, const Ice::Identity& /*identity*/, const string& /*facet*/,
                   const Ice::ObjectPtr& servant)
        {
            BetI* bet = dynamic_cast<BetI*>(servant.get());
            bet->init(_server._betEvictor, _server._bankEdge);
        }

    private:
        CasinoServer& _server;
    };

    _betEvictor =
        Freeze::createTransactionalEvictor(adapter, _envName, "bet",
                                           createTypeMap(CasinoStore::PersistentBet::ice_staticId()),
                                           new BetInitializer(*this));
    size = properties->getPropertyAsInt("Bet.EvictorSize");
    if(size > 0)
    {
        _betEvictor->setSize(size);
    }

    adapter->addServantLocator(_betEvictor, "bet");

    //
    // Prepare startup
    //

    try
    {
        _betResolver.start();

        //
        // Retrieve / create the bank
        //

        Ice::Identity bankId = communicator()->stringToIdentity("bank/Montecito");
        _bankPrx = CasinoStore::PersistentBankPrx::uncheckedCast(adapter->createProxy(bankId));

        if(!_bankEvictor->hasObject(bankId))
        {
            _bankEvictor->add(new BankI(_bankPrx, _bankEvictor, _playerEvictor, _betEvictor, _betResolver, _bankEdge),
                              bankId);
        }

        //
        // reload existing bets into the bet resolver
        //

        _bankPrx->reloadBets();

        //
        // Create players / recreate missing players missing players using a transaction
        // (the transaction is not really necessary here, but a good demo)
        //

        const string players[] =
        {
            "al", "bob", "charlie", "dave", "ed", "fred", "gene", "herb", "irvin", "joe", "ken", "lance"
        };

        Freeze::ConnectionPtr connection = Freeze::createConnection(communicator(), _envName);
        Freeze::TransactionPtr tx = connection->beginTransaction();
        
        _playerEvictor->setCurrentTransaction(tx);

        for(size_t i = 0; i < 12; ++i)
        {
            Ice::Identity ident = { players[i], "player" };
            if(!_playerEvictor->hasObject(ident))
            {
                _playerEvictor->add(new PlayerI, ident);
            }
        }

        tx->commit();
        assert(_playerEvictor->getCurrentTransaction() == 0);
        connection = 0;

        //
        // Everything is ready, activate
        //
        adapter->activate();

        shutdownOnInterrupt();
        communicator()->waitForShutdown();

        _betResolver.destroy();
    }
    catch(...)
    {
        _betResolver.destroy();
        throw;
    }
    return 0;
}

CasinoServer::CasinoServer(const string& envName) :
    _envName(envName)
{
}

map<string, string>
CasinoServer::createTypeMap(const string& defaultFacetType)
{
    map<string, string> result;
    result[""] = defaultFacetType;
    return result;
}
