// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


using System;

class CallbackClient : Ice.Application
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
			      + "v: set/reset override context field\n"
			      + "s: shutdown server\n"
			      + "x: exit\n"
			      + "?: help\n");
    }
    
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        string proxyProperty = "Callback.Client.Callback";
        string proxy = properties.getProperty(proxyProperty);
        if(proxy.Length == 0)
        {
            Console.Error.WriteLine("property `" + proxyProperty + "' not set");
            return 1;
        }
        
        Ice.ObjectPrx @base = communicator().stringToProxy(proxy);
        CallbackPrx twoway = CallbackPrxHelper.checkedCast(@base.ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }
        CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
        CallbackPrx batchOneway = CallbackPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        CallbackPrx datagram = CallbackPrxHelper.uncheckedCast(twoway.ice_datagram());
        CallbackPrx batchDatagram = CallbackPrxHelper.uncheckedCast(twoway.ice_batchDatagram());
        
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Client");
        adapter.add(new CallbackReceiverI(), Ice.Util.stringToIdentity("callbackReceiver"));
        adapter.activate();
        
        CallbackReceiverPrx twowayR = CallbackReceiverPrxHelper.uncheckedCast(
					adapter.createProxy(Ice.Util.stringToIdentity("callbackReceiver")));
        CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(
					twowayR.ice_oneway());
        //CallbackReceiverPrx batchOnewayR =
        //    CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_batchOneway());
        CallbackReceiverPrx datagramR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_datagram());
        //CallbackReceiverPrx batchDatagramR =
        //    CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_batchDatagram());
        
        string overwrite = null;
        
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
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "t";
                    twoway.initiateCallback(twowayR, context);
                }
                else if(line.Equals("o"))
                {
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "o";
                    oneway.initiateCallback(onewayR, context);
                }
                else if(line.Equals("O"))
                {
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "O";
                    if(overwrite != null)
                    {
                        context["_ovwt"] = overwrite;
                    }
                    batchOneway.initiateCallback(onewayR, context);
                }
                else if(line.Equals("d"))
                {
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "d";
                    datagram.initiateCallback(datagramR, context);
                }
                else if(line.Equals("D"))
                {
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "D";
                    if(overwrite != null)
                    {
                        context["_ovwt"] = overwrite;
                    }
                    batchDatagram.initiateCallback(datagramR, context);
                }
                else if(line.Equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(line.Equals("v"))
                {
                    if(overwrite == null)
                    {
                        overwrite = "some_value";
                        Console.Out.WriteLine("overwrite context field is now `" + overwrite + "'");
                    }
                    else
                    {
                        overwrite = null;
                        Console.Out.WriteLine("overwrite context field is empty");
                    }
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
}
