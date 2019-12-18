//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public sealed class TraceLevels
    {
        internal TraceLevels(Ice.Communicator communicator)
        {
            networkCat = "Network";
            protocolCat = "Protocol";
            retryCat = "Retry";
            locationCat = "Locator";
            slicingCat = "Slicing";
            threadPoolCat = "ThreadPool";

            string keyBase = "Ice.Trace.";

            network = communicator.GetPropertyAsInt(keyBase + networkCat) ?? 0;
            protocol = communicator.GetPropertyAsInt(keyBase + protocolCat) ?? 0;
            retry = communicator.GetPropertyAsInt(keyBase + retryCat) ?? 0;
            location = communicator.GetPropertyAsInt(keyBase + locationCat) ?? 0;
            slicing = communicator.GetPropertyAsInt(keyBase + slicingCat) ?? 0;
            threadPool = communicator.GetPropertyAsInt(keyBase + threadPoolCat) ?? 0;
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
