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
using System.ServiceModel.Description;

public class Client
{
    static void menu()
    {
        Console.WriteLine("usage:\n"
                       + "\n"
                       + "r: run latency test with selected transport\n"
                       + "\n"
                       + "select transport to use:\n"
                       + "1: TCP, binary encoded (default)\n"
                       + "2: HTTP, SOAP encoded\n"
                       + "\n"
                       + "other commands\n"
                       + "s: shutdown server\n"
                       + "x: exit\n"
                       + "?: help\n");
    }

    public static int Main(string[] args)
    {
        const int repetitions = 100000;

        string addr = "127.0.0.1";
        if(args.Length == 1)
        {
            addr = args[0];
        }
        else if(args.Length > 1)
        {
            Console.Error.WriteLine("usage: client [host|IP]");
            Environment.Exit(1);
        }

        Service.Latency tcpProxy = null;
        Service.Latency httpProxy = null;

        try
        {
            NetTcpBinding tcpBinding = new NetTcpBinding(SecurityMode.None);
            EndpointAddress tcpAddr = new EndpointAddress("net.tcp://" + addr + ":10001");
            ChannelFactory<Service.Latency> tcpFac = new ChannelFactory<Service.Latency>(tcpBinding, tcpAddr);
            tcpProxy = tcpFac.CreateChannel();

            WSHttpBinding httpBinding = new WSHttpBinding(SecurityMode.None);
            EndpointAddress httpAddr = new EndpointAddress("http://" + addr + ":10002");
            ChannelFactory<Service.Latency> httpFac = new ChannelFactory<Service.Latency>(httpBinding, httpAddr);
            httpProxy = httpFac.CreateChannel();
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            Environment.Exit(1);
        }

        menu();

        Service.Latency proxy = tcpProxy;

        String line = null;
        do
        {
            try
            {
                Console.Write("==> ");
                Console.Out.Flush();
                line = Console.ReadLine();
                if(line == null)
                {
                    break;
                }

                if(line.Equals("1"))
                {
                    Console.WriteLine("Using TCP");
                    proxy = tcpProxy;
                }
                else if(line.Equals("2"))
                {
                    Console.WriteLine("Using HTTP");
                    proxy = httpProxy;
                }
                else if(line.Equals("r"))
                {
                    Console.WriteLine("Pinging server " + repetitions + " times via " +
                        (proxy == tcpProxy ? "TCP" : "HTTP") + ". (This may take a while.)");
                    DateTime startTime = DateTime.Now;
                    for(int i = 0; i < repetitions; ++i)
                    {
                        proxy.o();
                    }
                    TimeSpan elapsed = DateTime.Now - startTime;
                    Console.WriteLine(elapsed.TotalMilliseconds + "ms" );
                }
                else if(line.Equals("s"))
                {
                    proxy.shutdown();
                }
                else if(line.Equals("x"))
                {
                    // Nothing to do, will exit below.
                }
                else if(line.Equals("?"))
                {
                    menu();
                }
                else
                {
                    Console.WriteLine("unknown command `" + line + "'");
                }
            }
            catch(Exception ex)
            {
                Console.Error.WriteLine(ex);
            }
        } while(!line.Equals("x"));

        return 0;
    }
}