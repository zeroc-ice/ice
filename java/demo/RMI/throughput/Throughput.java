// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface Throughput extends Remote
{
    int ByteSeqSize = 500000;

    int StringSeqSize = 50000;

    class StringDouble implements java.io.Serializable
    {
        public String s;
        public double d;
    };
    int StringDoubleSeqSize = 50000;

    class Fixed implements java.io.Serializable
    {
        int i;
        int j;
        double d;
    };
    int FixedSeqSize = 50000;

    void sendByteSeq(byte[] seq) throws RemoteException;
    byte[] recvByteSeq() throws RemoteException;
    byte[] echoByteSeq(byte[] seq) throws RemoteException;

    void sendStringSeq(String[] seq) throws RemoteException;
    String[] recvStringSeq() throws RemoteException;
    String[] echoStringSeq(String[] seq) throws RemoteException;

    void sendStructSeq(StringDouble[] seq) throws RemoteException;
    StringDouble[] recvStructSeq() throws RemoteException;
    StringDouble[] echoStructSeq(StringDouble[] seq) throws RemoteException;

    void sendFixedSeq(Fixed[] seq) throws RemoteException;
    Fixed[] recvFixedSeq() throws RemoteException;
    Fixed[] echoFixedSeq(Fixed[] seq) throws RemoteException;

    void shutdown() throws RemoteException;
}
