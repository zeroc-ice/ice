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
    public static java.nio.channels.SocketChannel
    createTcpSocket()
    {
        try
        {
            java.nio.channels.SocketChannel fd =
                java.nio.channels.SocketChannel.open();
            java.net.Socket socket = fd.socket();
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
            fd.configureBlocking(false);
            return fd;
        }
        catch (java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static java.nio.channels.DatagramChannel
    createUdpSocket()
    {
        try
        {
            return java.nio.channels.DatagramChannel.open();
        }
        catch (java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static void
    doConnect(java.nio.channels.SocketChannel fd,
              java.net.InetSocketAddress addr, int timeout)
    {
        try
        {
            if (!fd.connect(addr))
            {
                int delay;
                if (timeout > 0 && timeout < 100)
                {
                    delay = timeout;
                }
                else
                {
                    delay = 100; // 100 ms
                }

                int timer = 0;
                while (!fd.finishConnect())
                {
                    if (timeout > 0 && timer >= timeout)
                    {
                        fd.close();
                        throw new Ice.ConnectTimeoutException();
                    }
                    try
                    {
                        Thread.sleep(delay);
                        timer += delay;
                    }
                    catch (InterruptedException ex)
                    {
                    }
                }
            }
        }
        catch (java.net.ConnectException ex)
        {
            try
            {
                fd.close();
            }
            catch (java.io.IOException e)
            {
                // ignore
            }
            Ice.ConnectFailedException se = new Ice.ConnectFailedException();
            se.initCause(ex);
            throw se;
        }
        catch (java.io.IOException ex)
        {
            try
            {
                fd.close();
            }
            catch (java.io.IOException e)
            {
                // ignore
            }
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static void
    doConnect(java.nio.channels.DatagramChannel fd,
              java.net.InetSocketAddress addr, int timeout)
    {
        try
        {
            fd.connect(addr);
        }
        catch (java.net.ConnectException ex)
        {
            try
            {
                fd.close();
            }
            catch (java.io.IOException e)
            {
                // ignore
            }
            Ice.ConnectFailedException se = new Ice.ConnectFailedException();
            se.initCause(ex);
            throw se;
        }
        catch (java.io.IOException ex)
        {
            try
            {
                fd.close();
            }
            catch (java.io.IOException e)
            {
                // ignore
            }
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public static java.net.InetSocketAddress
    getAddress(String host, int port)
    {
        try
        {
            java.net.InetAddress addr = java.net.InetAddress.getByName(host);
            return new java.net.InetSocketAddress(addr, port);
        }
        catch (java.net.UnknownHostException ex)
        {
            throw new Ice.DNSException();
        }
    }

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

    public static String
    fdToString(java.nio.channels.SelectableChannel fd)
    {
        if (fd == null)
        {
            return "<closed>";
        }

        java.net.InetAddress localAddr =  null, remoteAddr = null;
        int localPort = -1, remotePort = -1;

        if (fd instanceof java.nio.channels.SocketChannel)
        {
            java.nio.channels.SocketChannel socketChannel =
                (java.nio.channels.SocketChannel)fd;
            java.net.Socket socket = socketChannel.socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        }
        else if (fd instanceof java.nio.channels.DatagramChannel)
        {
            java.nio.channels.DatagramChannel datagramChannel =
                (java.nio.channels.DatagramChannel)fd;
            java.net.DatagramSocket socket = datagramChannel.socket();
            localAddr = socket.getLocalAddress();
            localPort = socket.getLocalPort();
            remoteAddr = socket.getInetAddress();
            remotePort = socket.getPort();
        }
        else
        {
            assert(false);
        }

        StringBuffer s = new StringBuffer();
        s.append("local address = ");
        s.append(localAddr.getHostAddress());
        s.append(':');
        s.append(localPort);
        if (remoteAddr == null)
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
    addrToString(java.net.InetSocketAddress addr)
    {
        StringBuffer s = new StringBuffer();
        s.append(addr.getAddress().getHostAddress());
        s.append(':');
        s.append(addr.getPort());
        return s.toString();
    }
}
