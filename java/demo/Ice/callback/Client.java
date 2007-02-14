// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    class ShutdownHook extends Thread
    {
        public void
        run()
        {
            try
            {
                communicator().destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static void
    menu()
    {
        System.out.println(
            "usage:\n" +
            "t: send callback as twoway\n" +
            "o: send callback as oneway\n" +
            "O: send callback as batch oneway\n" +
            "d: send callback as datagram\n" +
            "D: send callback as batch datagram\n" +
            "f: flush all batch requests\n" +
            "S: switch secure mode on/off\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        //
        // Since this is an interactive demo we want to clear the
        // Application installed interrupt callback and install our
        // own shutdown hook.
        //
        setInterruptHook(new ShutdownHook());

        CallbackSenderPrx twoway = CallbackSenderPrxHelper.checkedCast(
            communicator().propertyToProxy("Callback.CallbackServer").
                ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }
        CallbackSenderPrx oneway = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_oneway());
        CallbackSenderPrx batchOneway = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        CallbackSenderPrx datagram = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_datagram());
        CallbackSenderPrx batchDatagram = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_batchDatagram());

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Client");
        adapter.add(new CallbackReceiverI(), communicator().stringToIdentity("callbackReceiver"));
        adapter.activate();

        CallbackReceiverPrx twowayR = 
            CallbackReceiverPrxHelper.uncheckedCast(adapter.createProxy(
                communicator().stringToIdentity("callbackReceiver")));
        CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());
        CallbackReceiverPrx datagramR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_datagram());

        boolean secure = false;
        String secureStr = "";

        menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        String line = null;
        do
        {
            try
            {
                System.out.print("==> ");
                System.out.flush();
                line = in.readLine();
                if(line == null)
                {
                    break;
                }
                if(line.equals("t"))
                {
                    twoway.initiateCallback(twowayR);
                }
                else if(line.equals("o"))
                {
                    oneway.initiateCallback(onewayR);
                }
                else if(line.equals("O"))
                {
                    batchOneway.initiateCallback(onewayR);
                }
                else if(line.equals("d"))
                {
                    if(secure)
                    {
                        System.out.println("secure datagrams are not supported");
                    }
                    else
                    {
                        datagram.initiateCallback(datagramR);
                    }
                }
                else if(line.equals("D"))
                {
                    if(secure)
                    {
                        System.out.println("secure datagrams are not supported");
                    }
                    else
                    {
                        batchDatagram.initiateCallback(datagramR);
                    }
                }
                else if(line.equals("S"))
                {
                    secure = !secure;
                    secureStr = secure ? "s" : "";

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
                        System.out.println("secure mode is now on");
                    }
                    else
                    {
                        System.out.println("secure mode is now off");
                    }
                }
                else if(line.equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(line.equals("s"))
                {
                    twoway.shutdown();
                }
                else if(line.equals("x"))
                {
                    // Nothing to do
                }
                else if(line.equals("?"))
                {
                    menu();
                }
                else
                {
                    System.out.println("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
        while(!line.equals("x"));

        return 0;
    }

    public static void
    main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }
}
