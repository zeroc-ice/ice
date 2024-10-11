//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public final class SysLoggerI implements Logger {
    public SysLoggerI(String prefix, String facilityString, String host, int port) {
        this(prefix, parseFacility(facilityString), parseHost(host), port);
    }

    private SysLoggerI(String prefix, int facility, InetAddress host, int port) {
        _prefix = prefix;
        _facility = facility;
        _host = host;
        _port = port;

        // Open a datagram socket to communicate with the localhost syslog daemon.
        try {
            _socket = new DatagramSocket();
            _socket.connect(_host, _port);
        } catch (IOException ex) {
            throw new SocketException(ex);
        }
    }

    @Override
    public void print(String message) {
        log(LOG_INFO, message);
    }

    @Override
    public void trace(String category, String message) {
        log(LOG_INFO, category + ": " + message);
    }

    @Override
    public void warning(String message) {
        log(LOG_WARNING, message);
    }

    @Override
    public void error(String message) {
        log(LOG_ERR, message);
    }

    @Override
    public String getPrefix() {
        return _prefix;
    }

    @Override
    public Logger cloneWithPrefix(String prefix) {
        return new SysLoggerI(prefix, _facility, _host, _port);
    }

    void destroy() {
        _socket.close();
    }

    private static int parseFacility(String facility) {
        if (facility.equals("LOG_KERN")) {
            return LOG_KERN;
        } else if (facility.equals("LOG_USER")) {
            return LOG_USER;
        } else if (facility.equals("LOG_MAIL")) {
            return LOG_MAIL;
        } else if (facility.equals("LOG_DAEMON")) {
            return LOG_DAEMON;
        } else if (facility.equals("LOG_AUTH")) {
            return LOG_AUTH;
        } else if (facility.equals("LOG_SYSLOG")) {
            return LOG_SYSLOG;
        } else if (facility.equals("LOG_LPR")) {
            return LOG_LPR;
        } else if (facility.equals("LOG_NEWS")) {
            return LOG_NEWS;
        } else if (facility.equals("LOG_UUCP")) {
            return LOG_UUCP;
        } else if (facility.equals("LOG_CRON")) {
            return LOG_CRON;
        } else if (facility.equals("LOG_AUTHPRIV")) {
            return LOG_AUTHPRIV;
        } else if (facility.equals("LOG_FTP")) {
            return LOG_FTP;
        } else if (facility.equals("LOG_LOCAL0")) {
            return LOG_LOCAL0;
        } else if (facility.equals("LOG_LOCAL1")) {
            return LOG_LOCAL1;
        } else if (facility.equals("LOG_LOCAL2")) {
            return LOG_LOCAL2;
        } else if (facility.equals("LOG_LOCAL3")) {
            return LOG_LOCAL3;
        } else if (facility.equals("LOG_LOCAL4")) {
            return LOG_LOCAL4;
        } else if (facility.equals("LOG_LOCAL5")) {
            return LOG_LOCAL5;
        } else if (facility.equals("LOG_LOCAL6")) {
            return LOG_LOCAL6;
        } else if (facility.equals("LOG_LOCAL7")) {
            return LOG_LOCAL7;
        } else {
            throw new InitializationException("Invalid value for Ice.SyslogFacility: " + facility);
        }
    }

    private static InetAddress parseHost(String host) {
        try {
            return InetAddress.getByName(host);
        } catch (Exception ex) {
            throw new InitializationException("Invalid value for Ice.SyslogHost: " + host);
        }
    }

    private void log(int severity, String message) {
        try {
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
        } catch (IOException ex) {
            throw new SocketException(ex);
        }
    }

    private String _prefix;
    private int _facility;
    private DatagramSocket _socket;
    private InetAddress _host;
    private int _port;

    //
    // Syslog facilities (as defined in syslog.h)
    //
    private static final int LOG_KERN = 0;
    private static final int LOG_USER = 1;
    private static final int LOG_MAIL = 2;
    private static final int LOG_DAEMON = 3;
    private static final int LOG_AUTH = 4;
    private static final int LOG_SYSLOG = 5;
    private static final int LOG_LPR = 6;
    private static final int LOG_NEWS = 7;
    private static final int LOG_UUCP = 8;
    private static final int LOG_CRON = 9;
    private static final int LOG_AUTHPRIV = 10;
    private static final int LOG_FTP = 11;
    private static final int LOG_LOCAL0 = 16;
    private static final int LOG_LOCAL1 = 17;
    private static final int LOG_LOCAL2 = 18;
    private static final int LOG_LOCAL3 = 19;
    private static final int LOG_LOCAL4 = 20;
    private static final int LOG_LOCAL5 = 21;
    private static final int LOG_LOCAL6 = 22;
    private static final int LOG_LOCAL7 = 23;

    //
    // Syslog priorities (as defined in syslog.h)
    //
    private static final int LOG_ERR = 3;
    private static final int LOG_WARNING = 4;
    private static final int LOG_INFO = 6;
}
