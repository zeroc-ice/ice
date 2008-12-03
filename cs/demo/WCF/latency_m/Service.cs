// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Service
{
    [System.ServiceModel.ServiceContract]
    interface Latency
    {
        [System.ServiceModel.OperationContract]
        void o();

        [System.ServiceModel.OperationContract]
        void shutdown();
    }
}
