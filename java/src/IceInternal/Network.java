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

public final class Network
{
    public static String
    getLocalHost(boolean numeric)
    {
        String host;

        try
        {
            if (!numeric)
            {
                host = java.net.InetAddress.getLocalHost().getHostName();
            }
            else
            {
                host = java.net.InetAddress.getLocalHost().getHostAddress();
            }
        }
        catch(java.net.UnknownHostException ex)
        {
            throw new Ice.DNSException();
        }

        return host;
    }
}
