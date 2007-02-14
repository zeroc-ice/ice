// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{ 
    static class ServantFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals("::Test::Servant"));
            Test._ServantTie tie = new Test._ServantTie();
            tie.ice_delegate(new ServantI(tie));
            return tie;
        }

        public void
        destroy()
        {
        }
    }

    static class FacetFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals("::Test::Facet"));
            Test._FacetTie tie = new Test._FacetTie();
            tie.ice_delegate(new FacetI(tie));
            return tie;
        }

        public void
        destroy()
        {
        }
    }

    static int
    run(String[] args, Ice.Communicator communicator, String envName)
    {
        communicator.getProperties().setProperty("Evictor.Endpoints", "default -p 12010");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Evictor");
    
        RemoteEvictorFactoryI factory = new RemoteEvictorFactoryI(adapter, envName);
        adapter.add(factory, communicator.stringToIdentity("factory"));
    
        Ice.ObjectFactory servantFactory = new ServantFactory();
        communicator.addObjectFactory(servantFactory, "::Test::Servant");

        Ice.ObjectFactory facetFactory = new FacetFactory();
        communicator.addObjectFactory(facetFactory, "::Test::Facet");
    
        adapter.activate();

        communicator.waitForShutdown();

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        String envName = "db";

        try
        {
            Ice.StringSeqHolder holder = new Ice.StringSeqHolder();
            holder.value = args;
            communicator = Ice.Util.initialize(holder);
            args = holder.value;
            status = run(args, communicator, envName);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
