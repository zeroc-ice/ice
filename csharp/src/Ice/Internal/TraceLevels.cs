// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class TraceLevels
{
    internal TraceLevels(Ice.Properties properties)
    {
        dispatchCat = "Dispatch";
        networkCat = "Network";
        protocolCat = "Protocol";
        retryCat = "Retry";
        locationCat = "Locator";
        slicingCat = "Slicing";
        threadPoolCat = "ThreadPool";

        string keyBase = "Ice.Trace.";

        dispatch = properties.getIcePropertyAsInt(keyBase + dispatchCat);
        network = properties.getIcePropertyAsInt(keyBase + networkCat);
        protocol = properties.getIcePropertyAsInt(keyBase + protocolCat);
        retry = properties.getIcePropertyAsInt(keyBase + retryCat);
        location = properties.getIcePropertyAsInt(keyBase + locationCat);
        slicing = properties.getIcePropertyAsInt(keyBase + slicingCat);
        threadPool = properties.getIcePropertyAsInt(keyBase + threadPoolCat);
    }

    public readonly int dispatch;
    public readonly string dispatchCat;
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
