// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    internal sealed class TraceLevels
    {
        internal const string LocatorCategory = "Ice.Trace.Locator";
        internal const string ProtocolCategory = "Ice.Trace.Protocol";
        internal const string RetryCategory = "Ice.Trace.Retry";
        internal const string SlicingCategory = "Ice.Trace.Slicing";
        internal const string TransportCategory = "Ice.Trace.Transport";

        internal readonly int Locator;
        internal readonly int Protocol;
        internal readonly int Retry;
        internal readonly int Slicing;
        internal readonly int Transport;

        internal TraceLevels(Communicator communicator)
        {
            Locator = communicator.GetPropertyAsInt(LocatorCategory) ?? 0;
            Protocol = communicator.GetPropertyAsInt(ProtocolCategory) ?? 0;
            Retry = communicator.GetPropertyAsInt(RetryCategory) ?? 0;
            Slicing = communicator.GetPropertyAsInt(SlicingCategory) ?? 0;
            Transport = communicator.GetPropertyAsInt(TransportCategory) ?? 0;
        }
    }
}
