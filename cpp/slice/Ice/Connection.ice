// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONNECTION_ICE
#define ICE_CONNECTION_ICE

module Ice
{

/**
 *
 * An Internet address, consisting of host and port information.
 *
 **/
local struct InternetAddress
{
    /** The "host" part of the Internet address **/
    string host;
    
    /** The port number part of the Internet address **/
    int port;
};

/**
 *
 * Base interface for querying specific information about particular
 * protocols.
 *
 **/
local interface ProtocolInfo
{
    /**
     *
     * Get the name of the protocol. For example, "tcp", "udp", or
     * "ssl".
     *
     * @return the name of the protocol.
     *
     **/
    string name();
};

/**
 *
 * Specific information about the TCP protocol.
 *
 **/
local interface TcpProtocolInfo extends ProtocolInfo
{
    // No entry yet
};

/**
 *
 * Specific information about the UDP protocol.
 *
 **/
local interface UdpProtocolInfo extends ProtocolInfo
{
    // No entry yet
};

/**
 *
 * Specific information about the SSL protocol.
 *
 **/
local interface SslProtocolInfo extends ProtocolInfo
{
    // No entry yet
};

/**
 *
 * A base interface providing generic information about Internet
 * connections.
 *
 * @see IncomingConnection
 * @see OutgoingConnection
 *
 **/
local interface Connection
{
    /**
     *
     * Get the local Internet address of the connection.
     *
     * @return The local Internet address.
     *
     **/
    InternetAddress getLocalAddress();

    /**
     *
     * Get the remote Internet address of the connection.
     *
     * @return The remote Internet address.
     *
     **/
    InternetAddress getRemoteAddress();

    /**
     *
     * Get protocol specific information.
     *
     * @return Protocol specific information.
     *
     **/
    ProtocolInfo getProtocolInfo();
};

/**
 *
 * A specialization of [Connection] for incoming connections accpeted
 * by a server.
 *
 * @see OutgoingConnection
 *
 **/
local interface IncomingConnection extends Connection
{
};

/**
 *
 * A specialization of [Connection] for outgoing connections
 * established by a client.
 *
 * @see IncomingConnection
 *
 **/
local interface OutgoingConnection extends Connection
{
};

};

#endif
