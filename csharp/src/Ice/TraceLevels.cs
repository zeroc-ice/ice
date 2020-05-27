//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    internal sealed class TraceLevels
    {
        internal TraceLevels(Communicator communicator)
        {
            NetworkCat = "Network";
            ProtocolCat = "Protocol";
            RetryCat = "Retry";
            LocationCat = "Locator";
            SlicingCat = "Slicing";

            string keyBase = "Ice.Trace.";

            Network = communicator.GetPropertyAsInt(keyBase + NetworkCat) ?? 0;
            Protocol = communicator.GetPropertyAsInt(keyBase + ProtocolCat) ?? 0;
            Retry = communicator.GetPropertyAsInt(keyBase + RetryCat) ?? 0;
            Location = communicator.GetPropertyAsInt(keyBase + LocationCat) ?? 0;
            Slicing = communicator.GetPropertyAsInt(keyBase + SlicingCat) ?? 0;
        }

        public readonly int Network;
        public readonly string NetworkCat;
        public readonly int Protocol;
        public readonly string ProtocolCat;
        public readonly int Retry;
        public readonly string RetryCat;
        public readonly int Location;
        public readonly string LocationCat;
        public readonly int Slicing;
        public readonly string SlicingCat;
    }
}
