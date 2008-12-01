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
    static int main(string[] args)
    {
        string addr = "127.0.0.1";
        if(args.Length == 1)
        {
            addr = args[0];
        }
        else if(args.Length > 1)
        {
            Console.Error.WriteLine("usage: server [host|IP]");
            Environment.Exit(1);
        }

        try
        {
            ServiceHost host = new ServiceHost(typeof(LatencyI));
            LatencyI._sync = host;

            Uri uri = new Uri("net.tcp://" + addr + ":10001");
            NetTcpBinding tcpBinding = new NetTcpBinding(SecurityMode.None);
            host.AddServiceEndpoint(typeof(Service.Latency), tcpBinding, uri);

            uri = new Uri("http://" + addr + ":10002");
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
