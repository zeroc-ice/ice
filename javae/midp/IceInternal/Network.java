// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Network
{
    public static boolean
    connectionLost(java.io.IOException ex)
    {
        //
        // TODO: The JDK raises a generic IOException for certain
        // cases of connection loss. Unfortunately, our only choice is
        // to search the exception message for distinguishing phrases.
        //

        String msg = ex.getMessage();

        if(msg != null)
        {
	    msg = msg.toLowerCase();
            final String[] msgs =
            {
                "connection reset by peer", // ECONNRESET
                "cannot send after socket shutdown", // ESHUTDOWN (Win32)
                "cannot send after transport endpoint shutdown", // ESHUTDOWN (Linux)
                "software caused connection abort", // ECONNABORTED
                "an existing connection was forcibly closed" // unknown
            };

            for(int i = 0; i < msgs.length; i++)
            {
                if(msg.indexOf(msgs[i]) != -1)
                {
                    return true;
                }
            }
        }

        return false;
    }

    public static boolean
    connectionRefused(java.io.IOException ex)
    {
        //
        // The JDK raises a generic ConnectException when the server
        // actively refuses a connection. Unfortunately, our only
        // choice is to search the exception message for
        // distinguishing phrases.
	//
	// TODO: Confirm actual message under MIDP
        //

        String msg = ex.getMessage();

        if(msg != null)
        {
	    msg = msg.toLowerCase();
            final String[] msgs =
            {
                "connection refused" // ECONNREFUSED
            };

            for(int i = 0; i < msgs.length; i++)
            {
                if(msg.indexOf(msgs[i]) != -1)
                {
                    return true;
                }
            }
        }

        return false;
    }
    
    public static boolean
    notConnected(java.io.IOException ex)
    {
	String msg = ex.getMessage().toLowerCase();
	if(msg.indexOf("transport endpoint is not connected") != -1)
	{
	    return true;
	}

	return false;
    }

    public static String
    getLocalHost(boolean numeric)
    {
	String result = System.getProperty("microedition.hostname");
	if(result == null)
	{
	    result = "127.0.0.1";
	}
	return result;
    }

    public static byte[]
    getLocalAddress()
    {
	byte[] b = new byte[4];
	b[0] = 127;
	b[1] = 0;
	b[2] = 0;
	b[3] = 1;
	return b;
    }

    public static String
    toString(javax.microedition.io.SocketConnection connection)
    {
	if(connection == null)
	{
	    return "<closed>";
	}

	try
	{
	    String localAddr = connection.getLocalAddress();
	    int localPort = connection.getLocalPort();
	    String remoteAddr = connection.getAddress();
	    int remotePort = connection.getPort();

	    return addressesToString(localAddr, localPort, remoteAddr, remotePort);
	}
	catch(java.io.IOException ex)
	{
	    return "<closed>";
	}
    }
    
    public static String
    toString(javax.microedition.io.ServerSocketConnection connection)
    {
	if(connection == null)
	{
	    return "<closed>";
	}

	try
	{
	    return connection.getLocalAddress() + ":" + connection.getLocalPort();
	}
	catch(java.io.IOException ex)
	{
	    return "<closed>";
	}
    }

    public static String
    addressesToString(String localAddr, int localPort, String remoteAddr, int remotePort)
    {
	StringBuffer s = new StringBuffer();
	s.append("local address = ");
	s.append(localAddr);
	s.append(':');
	s.append(localPort);
	if(remoteAddr == null || remoteAddr.length() == 0)
	{
	    s.append("\nremote address = <not connected>");
	}
	else
	{
	    s.append("\nremote address = ");
	    s.append(remoteAddr);
	    s.append(':');
	    s.append(remotePort);
	}

	return s.toString();
    }

    //
    // Attempt to parse a string containing an IP address into octets.
    //
    public static byte[]
    addrStringToIP(String str)
    {
	if(str.length() < "1.1.1.1".length())
	{
	    return null;
	}
	
	//
	// Copy to an array because it will make a few of the following operations more convenient.
	//
	char[] stringChars = str.toCharArray();
	StringBuffer[] octetStr = new StringBuffer[4];

	//
	// We need 4 octets so we are looking for three periods, if we don't have them we might as well return a
	// null, indicating failure.
	//
	int dotCount = 0;
	for(int i = 0; i < stringChars.length; ++i)
	{
	    if(stringChars[i] == '.')
	    {
		++dotCount;
		if(dotCount > 3)
		{
		    //
		    // Too many periods to be an IP address, treat as a hostname.
		    //
		    return null;
		}
	    }
	    else if(!Character.isDigit(stringChars[i]))
	    {
		return null;
	    }
	    else
	    {
		if(octetStr[dotCount] == null)
		{
		    octetStr[dotCount] = new StringBuffer(3);
		}
		octetStr[dotCount].append(stringChars[i]);
		if(octetStr[dotCount].length() > 3)
		{
		    return null;
		}
	    }
	}

	//
	// We didn't find enough periods for this to be an IP address.
	//
	if(dotCount != 3)
	{
	    return null;
	}

	byte[] ip = new byte[octetStr.length];
	for(int i = 0; i < octetStr.length; ++i)
	{
	    try
	    {
		Integer s = Integer.valueOf(octetStr[i].toString());
		if(s.intValue() < 0 || s.intValue() > 255)
		{
		    return null;
		}
		ip[i] = s.byteValue();
	    }
	    catch(NumberFormatException ex)
	    {
		return null;
	    }
	}
	return ip;
    }

    public static boolean
    interrupted(java.io.IOException ex)
    {
	return ex instanceof java.io.InterruptedIOException ||
	       ex.getMessage().indexOf("Interrupted system call") >= 0;
    }
}
