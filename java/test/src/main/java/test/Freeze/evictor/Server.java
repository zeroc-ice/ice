// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.evictor;
import test.Freeze.evictor.Test.*;

public class Server extends test.Util.Application
{
    static class AccountFactory implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals("::Test::Account"));
            return new AccountI();
        }

        public void
        destroy()
        {
        }
    }

    static class ServantFactory implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals("::Test::Servant"));
            _ServantTie tie = new _ServantTie();
            tie.ice_delegate(new ServantI(tie));
            return tie;
        }

        public void
        destroy()
        {
        }
    }

    static class FacetFactory implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals("::Test::Facet"));
            _FacetTie tie = new _FacetTie();
            tie.ice_delegate(new FacetI(tie));
            return tie;
        }

        public void
        destroy()
        {
        }
    }

    public int
    run(String[] args)
    {
        Ice.Communicator communicator = communicator();

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Evictor");

        communicator.addObjectFactory(new AccountFactory(), "::Test::Account");
        communicator.addObjectFactory(new ServantFactory(), "::Test::Servant");
        communicator.addObjectFactory(new FacetFactory(), "::Test::Facet");

        RemoteEvictorFactoryI factory = new RemoteEvictorFactoryI("db");
        adapter.add(factory, communicator.stringToIdentity("factory"));

        adapter.activate();

        communicator.waitForShutdown();

        return 0;
    }

    protected Ice.InitializationData
    getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Evictor.Endpoints", "default -p 12010");
        initData.properties.setProperty("Ice.Package.Test", "test.Freeze.evictor");
        return initData;
    }

    public static void
    main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);

        System.gc();
        System.exit(status);
    }
}
