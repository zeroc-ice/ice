// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;

public class Server implements Throughput
{

    public
    Server()
    {
        _byteSeq = new byte[ByteSeqSize];

        _stringSeq = new String[StringSeqSize];
        for(int i = 0; i < StringSeqSize; ++i)
        {
            _stringSeq[i] = new String("hello");
        }

        _structSeq = new StringDouble[StringDoubleSeqSize];
        for(int i = 0; i < StringDoubleSeqSize; ++i)
        {
            _structSeq[i] = new StringDouble();
            _structSeq[i].s = new String("hello");
            _structSeq[i].d = 3.14;
        }

        _fixedSeq = new Fixed[FixedSeqSize];
        for(int i = 0; i < FixedSeqSize; ++i)
        {
            _fixedSeq[i] = new Fixed();
            _fixedSeq[i].i = 0;
            _fixedSeq[i].j = 0;
            _fixedSeq[i].d = 0;
        }
    }

    public void
    sendByteSeq(byte[] seq)
    {
    }

    public byte[]
    recvByteSeq()
    {
        return _byteSeq;
    }

    public byte[]
    echoByteSeq(byte[] seq)
    {
        return seq;
    }

    public void
    sendStringSeq(String[] seq)
    {
    }

    public String[]
    recvStringSeq()
    {
        return _stringSeq;
    }

    public String[]
    echoStringSeq(String[] seq)
    {
        return seq;
    }

    public void
    sendStructSeq(StringDouble[] seq)
    {
    }

    public StringDouble[]
    recvStructSeq()
    {
        return _structSeq;
    }

    public StringDouble[]
    echoStructSeq(StringDouble[] seq)
    {
        return seq;
    }

    public void
    sendFixedSeq(Fixed[] seq)
    {
    }

    public Fixed[]
    recvFixedSeq()
    {
        return _fixedSeq;
    }

    public Fixed[]
    echoFixedSeq(Fixed[] seq)
    {
        return seq;
    }

    public void
    shutdown()
    {
        System.exit(0);
    }

    private byte[] _byteSeq;
    private String[] _stringSeq;
    private StringDouble[] _structSeq;
    private Fixed[] _fixedSeq;

    public static void main(String[] args)
    {
        if(System.getSecurityManager() == null)
        {
            System.setSecurityManager(new SecurityManager());
        }
        try
        {
            Throughput servant = new Server();
            Throughput stub = (Throughput)UnicastRemoteObject.exportObject(servant, 0);
            Registry registry = LocateRegistry.getRegistry();
            registry.rebind("throughput", stub);
            System.out.println("ready");
        }
        catch(Exception e)
        {
            System.err.println(e);
        }
    }
}
