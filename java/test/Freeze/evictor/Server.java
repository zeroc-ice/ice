// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
            return new ServantI();
        }

        public void
        destroy()
        {
        }
    }

    static int
    run(String[] args, Ice.Communicator communicator, Freeze.DBEnvironment dbEnv)
    {
        communicator.getProperties().setProperty("Evictor.Endpoints", "default -p 12345 -t 2000");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Evictor");
    
        RemoteEvictorFactoryI factory = new RemoteEvictorFactoryI(adapter, dbEnv);
        adapter.add(factory, Ice.Util.stringToIdentity("factory"));
    
        Ice.ObjectFactory servantFactory = new ServantFactory();
        communicator.addObjectFactory(servantFactory, "::Test::Servant");
    
        adapter.activate();

        communicator.waitForShutdown();

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        Freeze.DBEnvironment dbEnv = null;
        String dbEnvDir = "db";

        try
        {
            Ice.StringSeqHolder holder = new Ice.StringSeqHolder();
            holder.value = args;
            communicator = Ice.Util.initialize(holder);
            args = holder.value;
            if(args.length > 0)
            {
                dbEnvDir = args[0];
                dbEnvDir += "/";
                dbEnvDir += "db";
            }
            dbEnv = Freeze.Util.initialize(communicator, dbEnvDir);
            status = run(args, communicator, dbEnv);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(dbEnv != null)
        {
            dbEnv.close();
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

        System.exit(status);
    }
}
