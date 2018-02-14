// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    SysLoggerI(String prefix, String facilityString)
    {
        int facility;
        if(facilityString.equals("LOG_KERN"))
        {
            facility = LOG_KERN;
        }
        else if(facilityString.equals("LOG_USER"))
        {
            facility = LOG_USER;
        }
        else if(facilityString.equals("LOG_MAIL"))
        {
            facility = LOG_MAIL;
        }
        else if(facilityString.equals("LOG_DAEMON"))
        {
            facility = LOG_DAEMON;
        }
        else if(facilityString.equals("LOG_AUTH"))
        {
            facility = LOG_AUTH;
        }
        else if(facilityString.equals("LOG_SYSLOG"))
        {
            facility = LOG_SYSLOG;
        }
        else if(facilityString.equals("LOG_LPR"))
        {
            facility = LOG_LPR;
        }
        else if(facilityString.equals("LOG_NEWS"))
        {
            facility = LOG_NEWS;
        }
        else if(facilityString.equals("LOG_UUCP"))
        {
            facility = LOG_UUCP;
        }
        else if(facilityString.equals("LOG_CRON"))
        {
            facility = LOG_CRON;
        }
        else if(facilityString.equals("LOG_AUTHPRIV"))
        {
            facility = LOG_AUTHPRIV;
        }
        else if(facilityString.equals("LOG_FTP"))
        {
            facility = LOG_FTP;
        }
        else if(facilityString.equals("LOG_LOCAL0"))
        {
            facility = LOG_LOCAL0;
        }
        else if(facilityString.equals("LOG_LOCAL1"))
        {
            facility = LOG_LOCAL1;
        }
        else if(facilityString.equals("LOG_LOCAL2"))
        {
            facility = LOG_LOCAL2;
        }
        else if(facilityString.equals("LOG_LOCAL3"))
        {
            facility = LOG_LOCAL3;
        }
        else if(facilityString.equals("LOG_LOCAL4"))
        {
            facility = LOG_LOCAL4;
        }
        else if(facilityString.equals("LOG_LOCAL5"))
        {
            facility = LOG_LOCAL5;
        }
        else if(facilityString.equals("LOG_LOCAL6"))
        {
            facility = LOG_LOCAL6;
        }
        else if(facilityString.equals("LOG_LOCAL7"))
        {
            facility = LOG_LOCAL7;
        }
        else
        {
            throw new Ice.InitializationException("Invalid value for Ice.SyslogFacility: " + facilityString);
        }
        initialize(prefix, facility);
    }

    private
    SysLoggerI(String prefix, int facility)
    {
        initialize(prefix, facility);
    }

    private void
    initialize(String prefix, int facility)
    {
        _prefix = prefix;
        _facility = facility;

        //
        // Open a datagram socket to communicate with the localhost
        // syslog daemon.
        //
        try
        {
            _host = IceInternal.Network.getLocalAddress(IceInternal.Network.EnableBoth);
            _socket = new DatagramSocket();
            _socket.connect(_host, _port);
        }
        catch(IOException ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    @Override
    public void
    print(String message)
    {
        log(LOG_INFO, message);
    }

    @Override
    public void
    trace(String category, String message)
    {
        log(LOG_INFO, category + ": " + message);
    }

    @Override
    public void
    warning(String message)
    {
        log(LOG_WARNING, message);
    }

    @Override
    public void
    error(String message)
    {
        log(LOG_ERR, message);
    }

    @Override
    public String
    getPrefix()
    {
        return _prefix;
    }

    @Override
    public Logger
    cloneWithPrefix(String prefix)
    {
        return new SysLoggerI(prefix, _facility);
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

            int priority = (_facility << 3) | severity;

            String msg = '<' + Integer.toString(priority) + '>' + _prefix + ": " + message;

            byte buf[] = msg.getBytes();
            DatagramPacket p = new DatagramPacket(buf, buf.length, _host, _port);
            _socket.send(p);
        }
        catch(IOException ex)
        {
            throw new Ice.SocketException(ex);
        }
    }

    private String _prefix;
    private int _facility;
    private DatagramSocket _socket;
    private InetAddress _host;
    private static int _port = 514;

    //
    // Syslog facilities (as defined in syslog.h)
    //
    private final static int LOG_KERN = 0;
    private final static int LOG_USER = 1;
    private final static int LOG_MAIL = 2;
    private final static int LOG_DAEMON = 3;
    private final static int LOG_AUTH = 4;
    private final static int LOG_SYSLOG = 5;
    private final static int LOG_LPR = 6;
    private final static int LOG_NEWS = 7;
    private final static int LOG_UUCP = 8;
    private final static int LOG_CRON = 9;
    private final static int LOG_AUTHPRIV = 10;
    private final static int LOG_FTP = 11;
    private final static int LOG_LOCAL0 = 16;
    private final static int LOG_LOCAL1 = 17;
    private final static int LOG_LOCAL2 = 18;
    private final static int LOG_LOCAL3 = 19;
    private final static int LOG_LOCAL4 = 20;
    private final static int LOG_LOCAL5 = 21;
    private final static int LOG_LOCAL6 = 22;
    private final static int LOG_LOCAL7 = 23;

    //
    // Syslog priorities (as defined in syslog.h)
    //
    private final static int LOG_ERR = 3;
    private final static int LOG_WARNING = 4;
    private final static int LOG_INFO = 6;
}
