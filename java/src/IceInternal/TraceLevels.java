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

final class TraceLevels
{
    public
    TraceLevels(Ice.Properties properties)
    {
        network = 0;
        networkCat = "Network";
        protocol = 0;
        protocolCat = "Protocol";
        retry = 0;
        retryCat = "Retry";
        security = 0;
        securityCat = "Security";

        String value;
        final String keyBase = "Ice.Trace.";

        value = properties.getProperty(keyBase + networkCat);
        if (value != null)
        {
            try
            {
                network = Integer.parseInt(value);
            }
            catch (NumberFormatException ex)
            {
            }
        }

        value = properties.getProperty(keyBase + protocolCat);
        if (value != null)
        {
            try
            {
                protocol = Integer.parseInt(value);
            }
            catch (NumberFormatException ex)
            {
            }
        }

        value = properties.getProperty(keyBase + retryCat);
        if (value != null)
        {
            try
            {
                retry = Integer.parseInt(value);
            }
            catch (NumberFormatException ex)
            {
            }
        }

        value = properties.getProperty(keyBase + securityCat);
        if (value != null)
        {
            try
            {
                security = Integer.parseInt(value);
            }
            catch (NumberFormatException ex)
            {
            }
        }
    }

    int network;
    String networkCat;
    int protocol;
    String protocolCat;
    int retry;
    String retryCat;
    int security;
    String securityCat;
}
