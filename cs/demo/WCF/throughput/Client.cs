// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.ServiceModel;
using System.ServiceModel.Description;
using Service;
using System.Collections;

class Client
{
    static void menu()
    {
        Console.WriteLine("usage:\n"
                           + "\n"
                           + "select transport to use:\n"
                           + "a: TCP, binary encoded (default)\n"
                           + "b: HTTP, SOAP encoded\n"
                           + "\n"
                           + "toggle type of data to send:\n"
                           + "1: sequence of bytes (default)\n"
                           + "2: sequence of strings (\"hello\")\n"
                           + "3: sequence of structs with a string (\"hello\") and a double\n"
                           + "4: sequence of structs with two ints and a double\n"
                           + "\n"
                           + "select test to run:\n"
                           + "t: Send sequence\n"
                           + "r: Receive sequence\n"
                           + "e: Echo (send and receive) sequence\n"
                           + "\n"
                           + "other commands\n"
                           + "s: shutdown server\n"
                           + "x: exit\n"
                           + "?: help\n");
    }

    public static int Main(string[] args)
    {
        const int repetitions = 100;

        string addr = null;
        string tcpPort = "10001";
        string httpPort = "10002";

        if (args.Length == 0)
        {
            addr = "127.0.0.1";
        }
        else if (args.Length == 1)
        {
            addr = args[0];
        }
        else if (args.Length == 3)
        {
            addr = args[0];
            tcpPort = args[1];
            httpPort = args[2];
        }
        else
        {
            Console.Error.WriteLine("usage: client [host [tcpPort httpPort]]");
            Environment.Exit(1);
        }

        Test tcpProxy = null;
        Test httpProxy = null;

        try
        {
            NetTcpBinding tcpBinding = new NetTcpBinding(SecurityMode.None);
            tcpBinding.MaxReceivedMessageSize = 10000000;
            tcpBinding.ReaderQuotas.MaxArrayLength = 1000000;
            EndpointAddress tcpAddr = new EndpointAddress("net.tcp://" + addr + ":" + tcpPort);
            ChannelFactory<Test> tcpFac = new ChannelFactory<Test>(tcpBinding, tcpAddr);
            IEnumerator e = tcpFac.Endpoint.Contract.Operations.GetEnumerator();
            while(e.MoveNext())
            {
                OperationDescription od = (OperationDescription)e.Current;
                DataContractSerializerOperationBehavior dataContractBehavior =
                    od.Behaviors.Find<DataContractSerializerOperationBehavior>();
                if(dataContractBehavior != null)
                {
                    dataContractBehavior.MaxItemsInObjectGraph = 1000000;
                }
            }
            tcpProxy = tcpFac.CreateChannel();

            WSHttpBinding httpBinding = new WSHttpBinding(SecurityMode.None);
            httpBinding.MaxReceivedMessageSize = 10000000;
            httpBinding.ReaderQuotas.MaxArrayLength = 1000000;
            EndpointAddress httpAddr = new EndpointAddress("http://" + addr + ":" + httpPort);
            ChannelFactory<Test> httpFac = new ChannelFactory<Test>(httpBinding, httpAddr);
            e = httpFac.Endpoint.Contract.Operations.GetEnumerator();
            while(e.MoveNext())
            {
                OperationDescription od = (OperationDescription)e.Current;
                DataContractSerializerOperationBehavior dataContractBehavior =
                    od.Behaviors.Find<DataContractSerializerOperationBehavior>();
                if(dataContractBehavior != null)
                {
                    dataContractBehavior.MaxItemsInObjectGraph = 1000000;
                }
            }
            httpProxy = httpFac.CreateChannel();
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            Environment.Exit(1);
        }

        byte[] byteSeq = new byte[Sizes.ByteSeqSize];

        string[] stringSeq = new string[Sizes.StringSeqSize];
        for (int i = 0; i < Sizes.StringSeqSize; ++i)
        {
            stringSeq[i] = new string('a', 5);
        }

        StringDouble[] structSeq = new StringDouble[Sizes.StringDoubleSeqSize];
        for (int i = 0; i < Sizes.StringDoubleSeqSize; ++i)
        {
            structSeq[i].s = new string('a', 5);
            structSeq[i].d = 3.14;
        }

        Fixed[] fixedSeq = new Fixed[Sizes.FixedSeqSize];
        for(int i = 0; i < Sizes.FixedSeqSize; ++i)
        {
    		
            fixedSeq[i].i = 0;
            fixedSeq[i].j = 0;
            fixedSeq[i].d = 0;
        }

        try
        {
            Console.Write("Warming up...");
            Console.Out.Flush();

            byte[] byteS = new byte[1];
            string[] stringS = new string[1];
            StringDouble[] structS = new StringDouble[1];
            Fixed[] fixedS = new Fixed[1];

            for (int i = 0; i < 10000; ++i)
            {
                tcpProxy.sendByteSeq(byteS);
                tcpProxy.recvByteSeq();
                tcpProxy.echoByteSeq(byteS);
                httpProxy.sendByteSeq(byteS);
                httpProxy.recvByteSeq();
                httpProxy.echoByteSeq(byteS);
            }

            for (int i = 0; i < 10000; ++i)
            {
                tcpProxy.sendStringSeq(stringS);
                tcpProxy.recvStringSeq();
                tcpProxy.echoStringSeq(stringS);
                httpProxy.sendStringSeq(stringS);
                httpProxy.recvStringSeq();
                httpProxy.echoStringSeq(stringS);
            }

            for (int i = 0; i < 10000; ++i)
            {
                tcpProxy.sendStructSeq(structS);
                tcpProxy.recvStructSeq();
                tcpProxy.echoStructSeq(structS);
                httpProxy.sendStructSeq(structS);
                httpProxy.recvStructSeq();
                httpProxy.echoStructSeq(structS);
            }

            for (int i = 0; i < 10000; ++i)
            {
                tcpProxy.sendFixedSeq(fixedS);
                tcpProxy.recvFixedSeq();
                tcpProxy.echoFixedSeq(fixedS);
                httpProxy.sendFixedSeq(fixedS);
                httpProxy.recvFixedSeq();
                httpProxy.echoFixedSeq(fixedS);
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex);
            Environment.Exit(1);
        }
        Console.WriteLine();

        menu();

        //
        // By default, use TCP with byte sequence.
        //
        Test proxy = tcpProxy;
        char currentType = '1';
        int seqSize = Sizes.ByteSeqSize;

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

                if(line.Equals("a"))
                {
                    Console.WriteLine("Using TCP");
                    proxy = tcpProxy;
                }
                else if(line.Equals("b"))
                {
                    Console.WriteLine("Using HTTP");
                    proxy = httpProxy;
                }
                else if(line.Equals("1") || line.Equals("2") || line.Equals("3") || line.Equals("4"))
                {
                    currentType = line[0];
                    switch(currentType)
                    {
                        case '1':
                        {
                            Console.WriteLine("using byte sequences");
                            seqSize = Sizes.ByteSeqSize;
                            break;
                        }
                        case '2':
                        {
                            Console.WriteLine("using string sequences");
                            seqSize = Sizes.StringSeqSize;
                            break;
                        }
                        case '3':
                        {
                            Console.WriteLine("using variable-length struct sequences");
                            seqSize = Sizes.StringDoubleSeqSize;
                            break;
                        }
                        case '4':
                        {
                            Console.WriteLine("using fixed-length struct sequences");
                            seqSize = Sizes.FixedSeqSize;
                            break;
                        }
                    }
                }
                else if(line.Equals("t") || line.Equals("r") || line.Equals("e"))
                {
                    char c = line[0];
                    switch (c)
                    {
                        case 't':
                        {
                            Console.Write("sending");
                            break;
                        }
                        case 'r':
                        {
                            Console.Write("receiving");
                            break;
                        }
                        case 'e':
                        {
                            Console.Write("sending and receiving");
                            break;
                        }
                    }

                    Console.Write(" " + repetitions);

                    switch(currentType)
                    {
                        case '1':
                        {
                            Console.Write(" byte");
                            break;
                        }
                        case '2':
                        {
                            Console.Write(" string");
                            break;
                        }
                        case '3':
                        {
                            Console.Write(" variable-length struct");
                            break;
                        }
                        case '4':
                        {
                            Console.Write(" fixed-length struct");
                            break;
                        }
                    }
                    Console.WriteLine(" sequences of size " + seqSize + " via " +
                                       (proxy == tcpProxy ? "TCP" : "HTTP") + "...");

                    DateTime startTime = DateTime.Now;
                    
                    for(int i = 0; i < repetitions; ++i)
                    {
                        switch(currentType)
                        {
                            case '1':
                            {
                                switch(c)
                                {
                                    case 't':
                                    {
                                        proxy.sendByteSeq(byteSeq);
                                        break;
                                    }
                                    case 'r':
                                    {
                                        proxy.recvByteSeq();
                                        break;
                                    }
                                    case 'e':
                                    {
                                        proxy.echoByteSeq(byteSeq);
                                        break;
                                    }
                                }
                                break;
                            }

                            case '2':
                            {
                                switch(c)
                                {
                                    case 't':
                                    {
                                        proxy.sendStringSeq(stringSeq);
                                        break;
                                    }
                                    case 'r':
                                    {
                                        proxy.recvStringSeq();
                                        break;
                                    }
                                    case 'e':
                                    {
                                        proxy.echoStringSeq(stringSeq);
                                        break;
                                    }
                                }
                                break;
                            }

                            case '3':
                            {
                                switch(c)
                                {
                                    case 't':
                                    {
                                        proxy.sendStructSeq(structSeq);
                                        break;
                                    }
                                    case 'r':
                                    {
                                        proxy.recvStructSeq();
                                        break;
                                    }
                                    case 'e':
                                    {
                                        proxy.echoStructSeq(structSeq);
                                        break;
                                    }
                                }
                                break;
                            }

                            case '4':
                            {
                                switch(c)
                                {
                                    case 't':
                                    {
                                        proxy.sendFixedSeq(fixedSeq);
                                        break;
                                    }
                                    case 'r':
                                    {
                                        proxy.recvFixedSeq();
                                        break;
                                    }
                                    case 'e':
                                    {
                                        proxy.echoFixedSeq(fixedSeq);
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }

                    TimeSpan elapsed = DateTime.Now - startTime;
                    Console.WriteLine("time for " + repetitions + " sequences: " + elapsed.TotalMilliseconds + "ms");
                    Console.WriteLine("time per sequence: " + ((double)(elapsed.TotalMilliseconds / repetitions)) + "ms");
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
