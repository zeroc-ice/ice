// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client : Ice.Application
{
    private static void menu()
    {
        Console.Out.WriteLine("usage:\n"
	                      + "t: send callback as twoway\n"
			      + "o: send callback as oneway\n"
			      + "O: send callback as batch oneway\n"
			      + "d: send callback as datagram\n"
			      + "D: send callback as batch datagram\n"
			      + "f: flush all batch requests\n"
			      + "s: shutdown server\n"
			      + "x: exit\n"
			      + "?: help\n");
    }
    
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        string proxyProperty = "Callback.Client.CallbackServer";
        string proxy = properties.getProperty(proxyProperty);
        if(proxy.Length == 0)
        {
            Console.Error.WriteLine("property `" + proxyProperty + "' not set");
            return 1;
        }
        
        CallbackSenderPrx twoway = CallbackSenderPrxHelper.checkedCast(
	    communicator().stringToProxy(proxy).ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }
        CallbackSenderPrx oneway = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_oneway());
        CallbackSenderPrx batchOneway = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        CallbackSenderPrx datagram = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_datagram());
        CallbackSenderPrx batchDatagram = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_batchDatagram());
        
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Client");
        adapter.add(new CallbackReceiverI(), Ice.Util.stringToIdentity("callbackReceiver"));
        adapter.activate();
        
        CallbackReceiverPrx twowayR = CallbackReceiverPrxHelper.uncheckedCast(
					adapter.createProxy(Ice.Util.stringToIdentity("callbackReceiver")));
        CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(
					twowayR.ice_oneway());
        CallbackReceiverPrx datagramR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_datagram());
        
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
                    twoway.initiateCallback(twowayR);
                }
                else if(line.Equals("o"))
                {
                    oneway.initiateCallback(onewayR);
                }
                else if(line.Equals("O"))
                {
                    batchOneway.initiateCallback(onewayR);
                }
                else if(line.Equals("d"))
                {
                    datagram.initiateCallback(datagramR);
                }
                else if(line.Equals("D"))
                {
                    batchDatagram.initiateCallback(datagramR);
                }
                else if(line.Equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(line.Equals("s"))
                {
                    twoway.shutdown();
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
            catch(System.Exception ex)
            {
	    	Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));
        
        return 0;
    }

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config");
        System.Environment.Exit(status);
    }
}
