// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public class Server
{
    static class ServantTie extends Test._ServantTie
    {
	public void
	__write(IceInternal.BasicStream os, boolean marshalFacets)
	{
	    ((ServantI)ice_delegate()).setLastSavedValue();
	    super.__write(os, marshalFacets);
	}
    }

    static class FacetTie extends Test._FacetTie
    {
	public void
	__write(IceInternal.BasicStream os, boolean marshalFacets)
	{
	    ((FacetI)ice_delegate()).setLastSavedValue();
	    super.__write(os, marshalFacets);
	}
    }
    
    static class ServantFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals("::Test::Servant"));
	    ServantTie tie = new ServantTie();
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
	    FacetTie tie = new FacetTie();
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
        communicator.getProperties().setProperty("Evictor.Endpoints", "default -p 12345 -t 2000");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Evictor");
    
        RemoteEvictorFactoryI factory = new RemoteEvictorFactoryI(adapter, envName);
        adapter.add(factory, Ice.Util.stringToIdentity("factory"));
    
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

        System.exit(status);
    }
}
