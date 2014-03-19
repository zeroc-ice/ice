// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

public class Collocated
{
    static Thread _clientThread;
    static int result;

    public static void
    main(String[] args)
    {
        final Client client = new Client();
        final Server server = new Server();
        Thread t = new Thread(new Runnable()
        {
            public void run()
            {
                String[] args =
                {
                    "--Ice.NullHandleAbort=1",
                    "--Ice.Warn.Connections=1",
                    "--Ice.ThreadPool.Server.Size=1",
                    "--Ice.ThreadPool.Server.SizeMax=3",
                    "--Ice.ThreadPool.Server.SizeWarn=0",
                    "--Ice.PrintAdapterReady=1",
                    "--Ice.ServerIdleTime=30",
                    "--Ice.Default.Host=127.0.0.1"
                };
                server.setServerReadyListener(new test.Util.Application.ServerReadyListener()
                {
                    public void serverReady()
                    {
                        _clientThread = new Thread(new Runnable()
                        {
                            public void run()
                            {
                                String[] args =
                                {
                                    "--Ice.NullHandleAbort=1", "--Ice.Warn.Connections=1", "--Ice.Default.Host=127.0.0.1"
                                };
                                client.main("Client", args);
                            }
                        });
                        _clientThread.start();
                    }
                });

                result = server.main("Server", args);
                if(_clientThread != null)
                {
                    while(_clientThread.isAlive())
                    {
                        try
                        {
                            _clientThread.join();
                        }
                        catch(InterruptedException e1)
                        {
                        }
                    }
                }
            }
        });
        t.start();
        try
        {
            t.join();
        }
        catch(InterruptedException ex)
        {
        }
        System.gc();
        System.exit(result);
    }
}
