// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

public class Collocated extends test.Util.Application
{
    static Thread _clientThread;
    static int result;

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.gc();
        System.exit(result);
    }

    @Override
    public int
    run(String[] args)
    {
        final Client client = new Client();
        client.setClassLoader(getClassLoader());
        client.setWriter(getWriter());
        final Server server = new Server();
        server.setClassLoader(getClassLoader());
        server.setWriter(getWriter());
        Thread t = new Thread(new Runnable()
        {
            @Override
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
                    @Override
                    public void serverReady()
                    {
                        _clientThread = new Thread(new Runnable()
                        {
                            @Override
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

        return 0;
    }
}
