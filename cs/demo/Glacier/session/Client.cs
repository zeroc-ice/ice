// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Demo;

public class Client
{
    private static void menu()
    {
        Console.Out.WriteLine(
		"usage:\n" +
		"t: send greeting as twoway\n" +
		"o: send greeting as oneway\n" +
		"O: send greeting as batch oneway\n" +
		"d: send greeting as datagram\n" +
		"D: send greeting as batch datagram\n" +
		"f: flush all batch requests\n" +
		"T: set a timeout\n" +
		"x: exit\n" +
		"?: help\n");
    }
    
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        //
        // Do Glacier setup.
        //
        string glacierStarterEndpointsProperty = "Glacier.Starter.Endpoints";
        string glacierStarterEndpoints = properties.getProperty(glacierStarterEndpointsProperty);
        Debug.Assert(glacierStarterEndpoints.Length != 0);
        Ice.ObjectPrx starterBase = communicator.stringToProxy("Glacier/starter:" + glacierStarterEndpoints);
        Glacier.StarterPrx starter = Glacier.StarterPrxHelper.checkedCast(starterBase);
        if(starter == null)
        {
            Console.Error.WriteLine("Client: endpoints `" + glacierStarterEndpoints +
		"' do not refer to a glacier router starter");
            return 1;
        }
        
        byte[] privateKey;
        byte[] publicKey;
        byte[] routerCert;
        
        Glacier.RouterPrx router = null;
        string id;
        string pw;
        
        while(true)
        {
            try
            {
                Console.Out.Write("user id: ");
                Console.Out.Flush();
                id = Console.In.ReadLine();
                Console.Out.Write("password: ");
                Console.Out.Flush();
                pw = Console.In.ReadLine();
                
                router = starter.startRouter(id, pw, out privateKey, out publicKey, out routerCert);
            }
            catch(Glacier.PermissionDeniedException)
            {
                Console.Error.WriteLine("password is invalid, try again");
                continue;
            }
	    catch(Exception ex)
	    {
	        Console.Error.WriteLine(ex);
	    }
            break;
        }
        
        communicator.setDefaultRouter(router);
        
        Glacier.SessionPrx session = null;
        try
        {
            session = router.createSession();
        }
        catch(Glacier.NoSessionManagerException ex)
        {
            Console.Error.WriteLine(ex);
            return 1;
        }
        
        HelloSessionPrx @base = HelloSessionPrxHelper.checkedCast(session);
        
        HelloSessionPrx twoway
	    = HelloSessionPrxHelper.checkedCast(@base.ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            Console.Error.WriteLine("invalid object reference");
            return 1;
        }
        HelloSessionPrx oneway = HelloSessionPrxHelper.uncheckedCast(twoway.ice_oneway());
        HelloSessionPrx batchOneway = HelloSessionPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        HelloSessionPrx datagram = HelloSessionPrxHelper.uncheckedCast(twoway.ice_datagram());
        HelloSessionPrx batchDatagram = HelloSessionPrxHelper.uncheckedCast(twoway.ice_batchDatagram());
        
        int timeout = -1;
        
        menu();
        
        string line = null;
        do 
        {
            try
            {
                Console.Out.Write("==> ");
                Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null)
                {
                    break;
                }
                if(line.Equals("t"))
                {
                    twoway.hello();
                }
                else if(line.Equals("o"))
                {
                    oneway.hello();
                }
                else if(line.Equals("O"))
                {
                    batchOneway.hello();
                }
                else if(line.Equals("d"))
                {
                    datagram.hello();
                }
                else if(line.Equals("D"))
                {
                    batchDatagram.hello();
                }
                else if(line.Equals("f"))
                {
                    communicator.flushBatchRequests();
                }
                else if(line.Equals("T"))
                {
                    if(timeout == -1)
                    {
                        timeout = 2000;
                    }
                    else
                    {
                        timeout = -1;
                    }
                    
                    twoway = HelloSessionPrxHelper.uncheckedCast(twoway.ice_timeout(timeout));
                    oneway = HelloSessionPrxHelper.uncheckedCast(oneway.ice_timeout(timeout));
                    batchOneway = HelloSessionPrxHelper.uncheckedCast(batchOneway.ice_timeout(timeout));
                    
                    if(timeout == -1)
                    {
                        Console.Out.WriteLine("timeout is now switched off");
                    }
                    else
                    {
                        Console.Out.WriteLine("timeout is now set to 2000ms");
                    }
                }
                else if(line.Equals("x"))
                {
                    // Nothing to do
                }
                else if(line.Equals("?"))
                {
                    menu();
                }
                else
                {
                    Console.Out.WriteLine("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(Exception ex)
            {
                Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));
        
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(ref args, properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
	    Console.Error.WriteLine(ex);
            status = 1;
        }
        
	if(communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(System.Exception ex)
	    {
		Console.Error.WriteLine(ex);
		status = 1;
	    }
	}
        
        Environment.Exit(status);
    }
}
