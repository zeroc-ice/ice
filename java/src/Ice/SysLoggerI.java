// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.io.IOException;

public final class SysLoggerI implements Logger
{
    public 
    SysLoggerI(String ident)
    {
        _ident = ident;

        //
        // Open a datagram socket to communicate with the localhost
        // syslog daemon.
        // 
        try
        {
            _host = IceInternal.Network.getLocalAddress();
            _socket = new DatagramSocket();
            _socket.connect(_host, _port);
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    public void
    print(String message)
    {
        log(LOG_INFO, message);
    }

    public void
    trace(String category, String message)
    {
        log(LOG_INFO, category + ": " + message);
    }

    public void
    warning(String message)
    {
        log(LOG_WARNING, message);
    }

    public void
    error(String message)
    {
        log(LOG_ERR, message);
    }

    private void
    log(int severity, String message)
    {
        try 
        {
            //
            // Create a syslog message as defined by the RFC 3164:
            // <PRI>HEADER MSG. PRI is the priority and is calculated
            // from the facility and the severity. We don't specify
            // the HEADER. MSG contains the identifier followed by a
            // colon character and the message.
            //

            int priority = (LOG_USER << 3) | severity;

            String msg = '<' + Integer.toString(priority) + '>' + _ident + ": " + message;

            byte buf[] = msg.getBytes();
            DatagramPacket p = new DatagramPacket(buf, buf.length, _host, _port);
            _socket.send(p);
        } 
        catch(java.io.IOException ex) 
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    private String _ident;
    private DatagramSocket _socket;
    private InetAddress _host;
    private static int _port = 514;

    //
    // Syslog facilities facilities (as defined in syslog.h)
    // 
    private final int LOG_USER = 1;

    //
    // Syslog priorities (as defined in syslog.h)
    // 
    private final int LOG_ERR = 3;
    private final int LOG_WARNING = 4;
    private final int LOG_INFO = 6;
}

