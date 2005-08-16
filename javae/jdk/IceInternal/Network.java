// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

        String msg = ex.getMessage().toLowerCase();

        if(msg != null)
        {
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
    connectionRefused(java.net.ConnectException ex)
    {
        //
        // The JDK raises a generic ConnectException when the server
        // actively refuses a connection. Unfortunately, our only
        // choice is to search the exception message for
        // distinguishing phrases.
        //

        String msg = ex.getMessage().toLowerCase();

        if(msg != null)
        {
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
    notConnected(java.net.SocketException ex)
    {
	String msg = ex.getMessage().toLowerCase();
	if(msg.indexOf("transport endpoint is not connected") != -1)
	{
	    return true;
	}

	return false;
    }

    public static void
    closeSocket(java.net.Socket fd)
    {
	try
	{
	    fd.close();
	}
	catch(java.io.IOException ex)
	{
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
	}
    }

    public static InetSocketAddress
    getAddress(String host, int port)
    {
        try
        {
            java.net.InetAddress addr = java.net.InetAddress.getByName(host);
            return new InetSocketAddress(addr, port);
        }
        catch(java.net.UnknownHostException ex)
        {
	    Ice.DNSException e = new Ice.DNSException();
	    e.host = host;
	    throw e;
        }
    }

    public static String
    getLocalHost(boolean numeric)
    {
	byte[] addr = getLocalAddress();
	StringBuffer buf = new StringBuffer();
	for(int i = 0; i < addr.length; ++i)
	{
	    if(i != 0)
	    {
		buf.append('.');
	    }
	    int b = addr[i];
	    if(b < 0)
	    {
		b += 256;
	    }
	    buf.append(Integer.toString(b));
	}
	return buf.toString();
    }

    public static byte[]
    getLocalAddress()
    {
        java.net.InetAddress addr = null;

        try
        {
            addr = java.net.InetAddress.getLocalHost();
        }
        catch(java.net.UnknownHostException ex)
        {
            //
            // May be raised on DHCP systems.
            //
        }
        catch(NullPointerException ex)
        {
            //
            // Workaround for bug in JDK.
            //
        }

	if(addr == null)
	{
	    try
	    {
		addr = java.net.InetAddress.getByName("127.0.0.1");
	    }
	    catch(java.net.UnknownHostException ex)
	    {
		Ice.DNSException e = new Ice.DNSException();
		e.host = "127.0.0.1";
		throw e;
	    }
	}

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(addr != null);
	}
	return addr.getAddress();
    }

    public static String
    fdToString(java.net.Socket fd)
    {
	if(fd == null)
	{
	    return "<closed>";
	}

	java.net.InetAddress localAddr = fd.getLocalAddress();
	int localPort = fd.getLocalPort();
	java.net.InetAddress remoteAddr = fd.getInetAddress();
	int remotePort = fd.getPort();

	return addressesToString(localAddr, localPort, remoteAddr, remotePort);
    }

    public static String
    addressesToString(java.net.InetAddress localAddr, int localPort, java.net.InetAddress remoteAddr, int remotePort)
    {
	StringBuffer s = new StringBuffer();
	s.append("local address = ");
	s.append(localAddr.getHostAddress());
	s.append(':');
	s.append(localPort);
	if(remoteAddr == null)
	{
	    s.append("\nremote address = <not connected>");
	}
	else
	{
	    s.append("\nremote address = ");
	    s.append(remoteAddr.getHostAddress());
	    s.append(':');
	    s.append(remotePort);
	}

	return s.toString();
    }

    public static String
    addrToString(InetSocketAddress addr)
    {
        StringBuffer s = new StringBuffer();
        s.append(addr.getAddress().getHostAddress());
        s.append(':');
        s.append(addr.getPort());
        return s.toString();
    }

    public static boolean
    interrupted(java.io.IOException ex)
    {
	return ex instanceof java.io.InterruptedIOException ||
	       ex.getMessage().indexOf("Interrupted system call") >= 0;
    }
}
