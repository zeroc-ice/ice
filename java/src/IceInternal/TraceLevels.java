// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class TraceLevels
{
    TraceLevels(Ice.Properties properties)
    {
        networkCat = "Network";
        protocolCat = "Protocol";
        retryCat = "Retry";
        securityCat = "Security";

        final String keyBase = "Ice.Trace.";

        network = properties.getPropertyAsInt(keyBase + networkCat);
	protocol = properties.getPropertyAsInt(keyBase + protocolCat);
        retry = properties.getPropertyAsInt(keyBase + retryCat);
        security = properties.getPropertyAsInt(keyBase + securityCat);
    }

    final public int network;
    final public String networkCat;
    final public int protocol;
    final public String protocolCat;
    final public int retry;
    final public String retryCat;
    final public int security;
    final public String securityCat;
}
