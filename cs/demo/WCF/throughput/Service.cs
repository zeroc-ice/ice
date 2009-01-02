// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


using System;
using System.Runtime;
using System.Runtime.Serialization;
using System.ServiceModel;

namespace Service
{
    public class Sizes
    {
        public const int ByteSeqSize = 500000;
        public const int StringSeqSize = 50000;
        public const int StringDoubleSeqSize = 50000;
        public const int FixedSeqSize = 50000;
    }

    [DataContract]
    public struct StringDouble
    {
        [DataMember] public string s;
        [DataMember] public double d;
    };

	
    [DataContract]
    struct Fixed
    {
        [DataMember] public int i;
        [DataMember] public int j;
        [DataMember] public double d;
    };

    [ServiceContract]
    interface Test
    {
        [OperationContract]
        void sendByteSeq(byte[] seq);
        [OperationContract]
        byte[] recvByteSeq();
        [OperationContract]
        byte[] echoByteSeq(byte[] seq);

        [OperationContract]
        void sendStringSeq(string[] seq);
        [OperationContract]
        string[] recvStringSeq();
        [OperationContract]
        string[] echoStringSeq(string[] seq);
        
        [OperationContract]
        void sendStructSeq(StringDouble[] seq);
        [OperationContract]
        StringDouble[] recvStructSeq();
        [OperationContract]
        StringDouble[] echoStructSeq(StringDouble[] seq);

        [OperationContract]
        void sendFixedSeq(Fixed[] seq);
        [OperationContract]
        Fixed[] recvFixedSeq();
        [OperationContract]
        Fixed[] echoFixedSeq(Fixed[] seq);

        [OperationContract]
        void shutdown();
    };
}
