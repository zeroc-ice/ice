// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    internal sealed class TraceLevels
    {
        internal readonly int Transport;
        internal readonly string TransportCategory;
        internal readonly int Protocol;
        internal readonly string ProtocolCategory;
        internal readonly int Retry;
        internal readonly string RetryCategory;
        internal readonly int Location;
        internal readonly string LocationCategory;
        internal readonly int Slicing;
        internal readonly string SlicingCategory;

        internal TraceLevels(Communicator communicator)
        {
            TransportCategory = "Transport";
            ProtocolCategory = "Protocol";
            RetryCategory = "Retry";
            LocationCategory = "Locator";
            SlicingCategory = "Slicing";

            string keyBase = "Ice.Trace.";

            // TODO: remove once all mappings support Ice.Trace.Transport
            int network = communicator.GetPropertyAsInt(keyBase + "Network") ?? 0;
            Transport = communicator.GetPropertyAsInt(keyBase + TransportCategory) ?? network;
            Protocol = communicator.GetPropertyAsInt(keyBase + ProtocolCategory) ?? 0;
            Retry = communicator.GetPropertyAsInt(keyBase + RetryCategory) ?? 0;
            Location = communicator.GetPropertyAsInt(keyBase + LocationCategory) ?? 0;
            Slicing = communicator.GetPropertyAsInt(keyBase + SlicingCategory) ?? 0;
        }
    }
}
