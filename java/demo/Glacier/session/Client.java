// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    private static void
    menu()
    {
        System.out.println(
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

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        //
        // Do Glacier setup.
        //
        final String glacierStarterEndpointsProperty = "Glacier.Starter.Endpoints";
        String glacierStarterEndpoints = properties.getProperty(glacierStarterEndpointsProperty);
        assert(glacierStarterEndpoints.length() != 0);
        Ice.ObjectPrx starterBase = communicator.stringToProxy("Glacier/starter:" + glacierStarterEndpoints);
        Glacier.StarterPrx starter = Glacier.StarterPrxHelper.checkedCast(starterBase);
        if(starter == null)
        {
            System.err.println("Client: endpoints `" + glacierStarterEndpoints +
                "' do not refer to a glacier router starter");
            return 1;
        }

        Ice.ByteSeqHolder privateKey = new Ice.ByteSeqHolder();
        Ice.ByteSeqHolder publicKey = new Ice.ByteSeqHolder();
        Ice.ByteSeqHolder routerCert = new Ice.ByteSeqHolder();

        Glacier.RouterPrx router = null;
        String id;
        String pw;

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        while(true)
        {
            try
            {
                System.out.print("user id: ");
                System.out.flush();
                id = in.readLine();
                System.out.print("password: ");
                System.out.flush();
                pw = in.readLine();

                router = starter.startRouter(id, pw, privateKey, publicKey, routerCert);
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
                return 1;
            }
            catch(Glacier.CannotStartRouterException ex)
            {
                System.err.println("Client: " + ex + ":\n" + ex.reason);
                return 1;
            }
            catch(Glacier.PermissionDeniedException ex)
            {
                System.err.println("password is invalid, try again");
                continue;
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
            ex.printStackTrace();
            return 1;
        }

        HelloSessionPrx base = HelloSessionPrxHelper.checkedCast(session);

        HelloSessionPrx twoway = HelloSessionPrxHelper.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            System.err.println("invalid object reference");
            return 1;
        }
        HelloSessionPrx oneway = HelloSessionPrxHelper.uncheckedCast(twoway.ice_oneway());
        HelloSessionPrx batchOneway = HelloSessionPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        HelloSessionPrx datagram = HelloSessionPrxHelper.uncheckedCast(twoway.ice_datagram());
        HelloSessionPrx batchDatagram = HelloSessionPrxHelper.uncheckedCast(twoway.ice_batchDatagram());

        int timeout = -1;

        menu();

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
                    twoway.hello();
                }
                else if(line.equals("o"))
                {
                    oneway.hello();
                }
                else if(line.equals("O"))
                {
                    batchOneway.hello();
                }
                else if(line.equals("d"))
                {
                    datagram.hello();
                }
                else if(line.equals("D"))
                {
                    batchDatagram.hello();
                }
                else if(line.equals("f"))
                {
		    communicator.flushBatchRequests();
                }
                else if(line.equals("T"))
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
                        System.out.println("timeout is now switched off");
                    }
                    else
                    {
                        System.out.println("timeout is now set to 2000ms");
                    }
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
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties(args);
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(args, properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        } finally {
	    if(communicator != null)
	    {
                communicator.destroy();
	    }
	}

        System.exit(status);
    }
}
