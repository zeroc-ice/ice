// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class Server extends Ice.Application
{
    static class ObjectFactory implements Ice.ObjectFactory
    {
        ObjectFactory(Class factoryClass)
        {
            _factoryClass = factoryClass;
        }

        public Ice.Object
        create(String type)
        {
            try
            {
                return (Ice.Object)_factoryClass.newInstance();
            }
            catch(InstantiationException ex)
            {
                throw new Ice.InitializationException(ex.toString());
            }
            catch(IllegalAccessException ex)
            {
                throw new Ice.InitializationException(ex.toString());
            }
        }

        public void
        destroy()
        {
        }

        private Class _factoryClass;
    }

    private java.util.Map<String, String>
    createTypeMap(String defaultFacetType)
    {
        java.util.Map<String, String> result = new java.util.HashMap<String, String>();
        result.put("", defaultFacetType);
        return result;
    }

    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Ice.Properties properties = communicator().getProperties();

        _bankEdge = properties.getPropertyAsInt("Bank.Edge");
        if(_bankEdge < 1)
        {
            _bankEdge = 1;
        }
        System.out.println("Bank edge is " + _bankEdge);

        //
        // Create an object adapter
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Casino");

        //
        // Register factories
        //
        communicator().addObjectFactory(new ObjectFactory(BankI.class), CasinoStore.PersistentBank.ice_staticId());
        communicator().addObjectFactory(new ObjectFactory(PlayerI.class), CasinoStore.PersistentPlayer.ice_staticId());
        communicator().addObjectFactory(new ObjectFactory(BetI.class), CasinoStore.PersistentBet.ice_staticId());

        //
        // Create evictors; each type gets its own type-specific evictor
        //

        //
        // Bank evictor
        //

        Freeze.ServantInitializer bankInitializer = new Freeze.ServantInitializer()
            {
                public void
                initialize(Ice.ObjectAdapter adapter, Ice.Identity identity, String facet, Ice.Object servant)
                {
                    ((BankI)servant).init(_bankPrx, _bankEvictor, _playerEvictor, _betEvictor, _betResolver, _bankEdge);
                }
            };

        _bankEvictor =
            Freeze.Util.createTransactionalEvictor(adapter, _envName, "bank",
                                                   createTypeMap(CasinoStore.PersistentBank.ice_staticId()),
                                                   bankInitializer, null, true);

        int size = properties.getPropertyAsInt("Bank.EvictorSize");
        if(size > 0)
        {
            _bankEvictor.setSize(size);
        }

        adapter.addServantLocator(_bankEvictor, "bank");

        //
        // Player evictor
        //

        Freeze.ServantInitializer playerInitializer = new Freeze.ServantInitializer()
            {
                public void
                initialize(Ice.ObjectAdapter adapter, Ice.Identity identity, String facet, Ice.Object servant)
                {
                    CasinoStore.PersistentPlayerPrx prx =
                        CasinoStore.PersistentPlayerPrxHelper.uncheckedCast(adapter.createProxy(identity));
                    ((PlayerI)servant).init(prx, _playerEvictor, _bankPrx);
                }
            };

        _playerEvictor =
            Freeze.Util.createTransactionalEvictor(adapter, _envName, "player",
                                                   createTypeMap(CasinoStore.PersistentPlayer.ice_staticId()),
                                                   playerInitializer, null, true);

        size = properties.getPropertyAsInt("Player.EvictorSize");
        if(size > 0)
        {
            _playerEvictor.setSize(size);
        }

        adapter.addServantLocator(_playerEvictor, "player");

        //
        // Bet evictor
        //

        Freeze.ServantInitializer betInitializer = new Freeze.ServantInitializer()
            {
                public void
                initialize(Ice.ObjectAdapter adapter, Ice.Identity identity, String facet, Ice.Object servant)
                {
                    ((BetI)servant).init(_betEvictor, _bankEdge);
                }
            };

        _betEvictor =
            Freeze.Util.createTransactionalEvictor(adapter, _envName, "bet",
                                                   createTypeMap(CasinoStore.PersistentBet.ice_staticId()),
                                                   betInitializer, null, true);
        size = properties.getPropertyAsInt("Bet.EvictorSize");
        if(size > 0)
        {
            _betEvictor.setSize(size);
        }

        adapter.addServantLocator(_betEvictor, "bet");

        //
        // Prepare startup
        //

        _betResolver = new BetResolver();

        try
        {
            //
            // Retrieve / create the bank
            //

            Ice.Identity bankId = Ice.Util.stringToIdentity("bank/Montecito");
            _bankPrx = CasinoStore.PersistentBankPrxHelper.uncheckedCast(adapter.createProxy(bankId));

            if(!_bankEvictor.hasObject(bankId))
            {
                _bankEvictor.add(
                    new BankI(_bankPrx, _bankEvictor, _playerEvictor, _betEvictor, _betResolver, _bankEdge),
                    bankId);
            }

            //
            // reload existing bets into the bet resolver
            //
            _bankPrx.reloadBets();
            
            //
            // Create players / recreate missing players using a transaction
            // (the transaction is not really necessary here, but a good demo)
            //

            String[] players =
                { "al", "bob", "charlie", "dave", "ed", "fred", "gene", "herb", "irvin", "joe", "ken", "lance" };

            
            Freeze.Connection connection = Freeze.Util.createConnection(communicator(), _envName);
            Freeze.Transaction tx = connection.beginTransaction();

            _playerEvictor.setCurrentTransaction(tx);

            for(String player : players)
            {
                Ice.Identity ident = new Ice.Identity(player, "player");
                if(!_playerEvictor.hasObject(ident))
                {
                    _playerEvictor.add(new PlayerI(), ident);
                }
            }

            tx.commit();
            assert(_playerEvictor.getCurrentTransaction() == null);
            connection.close();
            

            //
            // Everything is ready, activate
            //
            adapter.activate();

            shutdownOnInterrupt();
            communicator().waitForShutdown();
            defaultInterrupt();
        }
        finally
        {
            _betResolver.cancel();
        }
        return 0;
    }

    Server(String envName)
    {
        _envName = envName;
    }

    static public void
    main(String[] args)
    {
        Server app = new Server("db");
        app.main("demo.Freeze.casino.Server", args, "config.server");
    }

    private String _envName;

    private CasinoStore.PersistentBankPrx _bankPrx;
    private Freeze.TransactionalEvictor _bankEvictor;
    private Freeze.TransactionalEvictor _playerEvictor;
    private Freeze.TransactionalEvictor _betEvictor;
    private BetResolver _betResolver;
    private int _bankEdge;
}
