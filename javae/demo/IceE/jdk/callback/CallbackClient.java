// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class CallbackClient extends Ice.Application
{
    private static void
    menu()
    {
        System.out.println(
            "usage:\n" +
            "t: send callback as twoway\n" +
            "o: send callback as oneway\n" +
            "O: send callback as batch oneway\n" +
            "f: flush all batch requests\n" +
	    "S: switch secure mode on/off\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        final String proxyProperty = "Callback.Client.CallbackServer";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
        {
            System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator().stringToProxy(proxy);
        CallbackPrx twoway = CallbackPrxHelper.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }
        CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
        CallbackPrx batchOneway = CallbackPrxHelper.uncheckedCast(twoway.ice_batchOneway());

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Client");
        adapter.add(new CallbackReceiverI(), Ice.Util.stringToIdentity("callbackReceiver"));
        adapter.activate();

        CallbackReceiverPrx twowayR = 
	    CallbackReceiverPrxHelper.uncheckedCast(adapter.createProxy(
                Ice.Util.stringToIdentity("callbackReceiver")));
        CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());

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
		else if(line.equals("S"))
		{
		    secure = !secure;
		    secureStr = secure ? "s" : "";

		    twoway = CallbackPrxHelper.uncheckedCast(twoway.ice_secure(secure));
		    oneway = CallbackPrxHelper.uncheckedCast(oneway.ice_secure(secure));
		    batchOneway = CallbackPrxHelper.uncheckedCast(batchOneway.ice_secure(secure));

		    twowayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_secure(secure));
		    onewayR = CallbackReceiverPrxHelper.uncheckedCast(onewayR.ice_secure(secure));

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
}
