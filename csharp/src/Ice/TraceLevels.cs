// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    public sealed class TraceLevels
    {
        internal TraceLevels(Ice.Properties properties)
        {
            networkCat = "Network";
            protocolCat = "Protocol";
            retryCat = "Retry";
            locationCat = "Locator";
            slicingCat = "Slicing";
            threadPoolCat = "ThreadPool";

            string keyBase = "Ice.Trace.";

            network = properties.getPropertyAsInt(keyBase + networkCat);
            protocol = properties.getPropertyAsInt(keyBase + protocolCat);
            retry = properties.getPropertyAsInt(keyBase + retryCat);
            location = properties.getPropertyAsInt(keyBase + locationCat);
            slicing = properties.getPropertyAsInt(keyBase + slicingCat);
            threadPool = properties.getPropertyAsInt(keyBase + threadPoolCat);
        }

        public readonly int network;
        public readonly string networkCat;
        public readonly int protocol;
        public readonly string protocolCat;
        public readonly int retry;
        public readonly string retryCat;
        public readonly int location;
        public readonly string locationCat;
        public readonly int slicing;
        public readonly string slicingCat;
        public readonly int threadPool;
        public readonly string threadPoolCat;
    }

}
