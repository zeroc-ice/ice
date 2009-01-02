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
using System.ServiceModel.Channels;
using System.Threading;
using Service;
using System.Collections;

class TestI : Service.Test
{
    TestI()
    {
        _byteSeq = new byte[Sizes.ByteSeqSize];

        _stringSeq = new string[Sizes.StringSeqSize];
        for(int i = 0; i < Sizes.StringSeqSize; ++i)
        {
            _stringSeq[i] = new string('a', 5);
        }

        _structSeq = new StringDouble[Sizes.StringDoubleSeqSize];
        for(int i = 0; i < Sizes.StringDoubleSeqSize; ++i)
        {
            _structSeq[i].s = new string('a', 5);
            _structSeq[i].d = 3.14;
        }

        _fixedSeq = new Fixed[Sizes.FixedSeqSize];
        for (int i = 0; i < Sizes.FixedSeqSize; ++i)
        {
			
            _fixedSeq[i].i = 0;
            _fixedSeq[i].j = 0;
            _fixedSeq[i].d = 0;
        }
    }

    public virtual void sendByteSeq(byte[] seq)
    {
    }

    public virtual byte[] recvByteSeq()
    {
        return _byteSeq;
    }

    public virtual byte[] echoByteSeq(byte[] seq)
    {
        return seq;
    }

    public virtual void sendStringSeq(string[] seq)
    {
    }

    public virtual string[] recvStringSeq()
    {
        return _stringSeq;
    }

    public virtual string[] echoStringSeq(string[] seq)
    {
        return seq;
    }

    public virtual void sendStructSeq(StringDouble[] seq)
    {
    }

    public virtual StringDouble[] recvStructSeq()
    {
        return _structSeq;
    }

    public virtual StringDouble[] echoStructSeq(StringDouble[] seq)
    {
        return seq;
    }

    public virtual void sendFixedSeq(Fixed[] seq)
    {
    }

    public virtual Fixed[] recvFixedSeq()
    {
        return _fixedSeq;
    }

    public virtual Fixed[] echoFixedSeq(Fixed[] seq)
    {
        return seq;
    }

    public virtual void shutdown()
    {
        Monitor.Enter(_sync);
        Monitor.Pulse(_sync);
        Monitor.Exit(_sync);
    }

    public static object _sync;

    private byte[] _byteSeq;
    private string[] _stringSeq;
    private StringDouble[] _structSeq;
    private Fixed[] _fixedSeq;
};

class Server
{
    public static int Main(string[] args)
    {
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
            Console.Error.WriteLine("usage: server [host [tcpPort httpPort]]");
            Environment.Exit(1);
        }

        ServiceHost host = new ServiceHost(typeof(TestI)); 
        TestI._sync = host;

	    Uri tcpUri = new Uri("net.tcp://" + addr + ":" + tcpPort);
        NetTcpBinding tcpBinding = new NetTcpBinding(SecurityMode.None);
        tcpBinding.MaxReceivedMessageSize = 10000000;
        tcpBinding.ReaderQuotas.MaxArrayLength = 1000000;
        ServiceEndpoint ep = host.AddServiceEndpoint(typeof(Test), tcpBinding, tcpUri);
        IEnumerator e = ep.Contract.Operations.GetEnumerator();
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

	    Uri httpUri = new Uri("http://" + addr + ":" + httpPort);
        WSHttpBinding httpBinding = new WSHttpBinding(SecurityMode.None);
        httpBinding.MaxReceivedMessageSize = 10000000;
        httpBinding.ReaderQuotas.MaxArrayLength = 1000000;
        ep = host.AddServiceEndpoint(typeof(Test), httpBinding, httpUri);
        e = ep.Contract.Operations.GetEnumerator();
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

        Monitor.Enter(host);
        host.Open();
        Monitor.Wait(host);
        Monitor.Exit(host);
        host.Close();
        return 0;
    }
}
