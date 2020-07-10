//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    internal sealed class TraceLevels
    {
        internal TraceLevels(Communicator communicator)
        {
            NetworkCategory = "Network";
            ProtocolCategory = "Protocol";
            RetryCategory = "Retry";
            LocationCategory = "Locator";
            SlicingCategory = "Slicing";

            string keyBase = "Ice.Trace.";

            Network = communicator.GetPropertyAsInt(keyBase + NetworkCategory) ?? 0;
            Protocol = communicator.GetPropertyAsInt(keyBase + ProtocolCategory) ?? 0;
            Retry = communicator.GetPropertyAsInt(keyBase + RetryCategory) ?? 0;
            Location = communicator.GetPropertyAsInt(keyBase + LocationCategory) ?? 0;
            Slicing = communicator.GetPropertyAsInt(keyBase + SlicingCategory) ?? 0;
        }

        public readonly int Network;
        public readonly string NetworkCategory;
        public readonly int Protocol;
        public readonly string ProtocolCategory;
        public readonly int Retry;
        public readonly string RetryCategory;
        public readonly int Location;
        public readonly string LocationCategory;
        public readonly int Slicing;
        public readonly string SlicingCategory;
    }
}
