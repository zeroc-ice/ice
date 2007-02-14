// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        Console.Out.Write("usage:\n"
                          + "t: send callback as twoway\n"
                          + "o: send callback as oneway\n"
                          + "O: send callback as batch oneway\n"
                          + "d: send callback as datagram\n"
                          + "D: send callback as batch datagram\n"
                          + "f: flush all batch requests");
        if(_haveSSL)
        {
            Console.Out.Write("\nS: switch secure mode on/off");
        }
        Console.Out.WriteLine("\ns: shutdown server\n"
                              + "x: exit\n"
                              + "?: help\n");
    }
    
    public override int run(string[] args)
    {
        try
        {
            communicator().getPluginManager().getPlugin("IceSSL");
            _haveSSL = true;
        }
        catch(Ice.NotRegisteredException)
        {
        }

        CallbackSenderPrx twoway = CallbackSenderPrxHelper.checkedCast(
            communicator().propertyToProxy("Callback.CallbackServer").
                ice_twoway().ice_timeout(-1).ice_secure(false));
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
        adapter.add(new CallbackReceiverI(), communicator().stringToIdentity("callbackReceiver"));
        adapter.activate();
        
        CallbackReceiverPrx twowayR = CallbackReceiverPrxHelper.uncheckedCast(
                                        adapter.createProxy(communicator().stringToIdentity("callbackReceiver")));
        CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(
                                        twowayR.ice_oneway());
        CallbackReceiverPrx datagramR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_datagram());
        
        bool secure = false;

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
                    if(secure)
                    {
                        Console.Out.WriteLine("secure datagrams are not supported");
                    }
                    else
                    {
                        datagram.initiateCallback(datagramR);
                    }
                }
                else if(line.Equals("D"))
                {
                    if(secure)
                    {
                        Console.Out.WriteLine("secure datagrams are not supported");
                    }
                    else
                    {
                        batchDatagram.initiateCallback(datagramR);
                    }
                }
                else if(line.Equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(_haveSSL && line.Equals("S"))
                {
                    secure = !secure;

                    twoway = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_secure(secure));
                    oneway = CallbackSenderPrxHelper.uncheckedCast(oneway.ice_secure(secure));
                    batchOneway = CallbackSenderPrxHelper.uncheckedCast(batchOneway.ice_secure(secure));
                    datagram = CallbackSenderPrxHelper.uncheckedCast(datagram.ice_secure(secure));
                    batchDatagram = CallbackSenderPrxHelper.uncheckedCast(batchDatagram.ice_secure(secure));

                    twowayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_secure(secure));
                    onewayR = CallbackReceiverPrxHelper.uncheckedCast(onewayR.ice_secure(secure));
                    datagramR = CallbackReceiverPrxHelper.uncheckedCast(datagramR.ice_secure(secure));

                    if(secure)
                    {
                        Console.Out.WriteLine("secure mode is now on");
                    }
                    else
                    {
                        Console.Out.WriteLine("secure mode is now off");
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

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }

    private static bool _haveSSL = false;
}
