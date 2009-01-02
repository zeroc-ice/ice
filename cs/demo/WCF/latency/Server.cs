// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.ServiceModel;
using System.Threading;

class LatencyI : Service.Latency
{
    public virtual void o()
    {
    }

    public virtual void shutdown()
    {
        Monitor.Enter(_sync);
        Monitor.Pulse(_sync);
        Monitor.Exit(_sync);
    }

    public static Object _sync;
};

class Server
{
    public static int Main(string[] args)
    {
        string addr = null;
        string tcpPort = "10001";
        string httpPort = "10002";

        if(args.Length == 0)
        {
            addr = "127.0.0.1";
        }
        else if(args.Length == 1)
        {
            addr = args[0];
        }
        else if(args.Length == 3)
        {
            addr = args[0];
            tcpPort = args[1];
            httpPort = args[2];
        }
        else
        {
            Console.Error.WriteLine("usage: server [host [tcpPort httpPort]]");
            Environment.Exit(1);
        }

        try
        {
            ServiceHost host = new ServiceHost(typeof(LatencyI));
            LatencyI._sync = host;

            Uri uri = new Uri("net.tcp://" + addr + ":" + tcpPort);
            NetTcpBinding tcpBinding = new NetTcpBinding(SecurityMode.None);
            host.AddServiceEndpoint(typeof(Service.Latency), tcpBinding, uri);

            uri = new Uri("http://" + addr + ":" + httpPort);
            WSHttpBinding httpBinding = new WSHttpBinding(SecurityMode.None);
            host.AddServiceEndpoint(typeof(Service.Latency), httpBinding, uri);

            Monitor.Enter(host);
            host.Open();
            Monitor.Wait(host);
            Monitor.Exit(host);
            host.Close();
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            Environment.Exit(1);
        }
        return 0;
    }
}
