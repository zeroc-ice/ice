// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
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
	locationCat = "Location";

        final String keyBase = "Ice.Trace.";

        network = properties.getPropertyAsInt(keyBase + networkCat);
	protocol = properties.getPropertyAsInt(keyBase + protocolCat);
        retry = properties.getPropertyAsInt(keyBase + retryCat);
        security = properties.getPropertyAsInt(keyBase + securityCat);
	location = properties.getPropertyAsInt(keyBase + locationCat);
    }

    final public int network;
    final public String networkCat;
    final public int protocol;
    final public String protocolCat;
    final public int retry;
    final public String retryCat;
    final public int security;
    final public String securityCat;
    final public int location;
    final public String locationCat;
}
