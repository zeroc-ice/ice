// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ADMIN_ICE
#define ICE_PACK_ADMIN_ICE

#include <Ice/Identity.ice>

/**
 *
 * The &Ice; module for object location and activation.
 *
 **/
module IcePack
{

/**
 *
 * A vector of strings representing command line arguments.
 *
 **/
sequence<string> Args;

/**
 *
 * Data describing a server and one or more objects implemented by that
 * server.
 *
 **/
struct ServerDescription
{
    /**
     *
     * The server object or object template. Any non-administrative
     * &IcePack; request that matches the identity of [object] will be
     * forwarded to [object].
     *
     **/
    Object* object;

    /**
     *
     * The optional server host name. If none is given, no automatic
     * activation will be performed.
     *
     * @see path
     *
     **/
    string host;

    /**
     *
     * The optional server path. If none is given, no automatic
     * activation will be performed.
     *
     * @see host
     * @see args
     *
     **/
    string path;

    /**
     *
     * The optional server arguments.
     *
     * @see path
     *
     **/
    Args args;
};

/**
 *
 * A dictionary of server descriptions. The dictionary key is the &Ice;
 * Object identity, and the value is the corresponding server
 * description.
 *
 **/
dictionary<Ice::Identity, ServerDescription> ServerDescriptions;

/**
 *
 * The &IcePack; administrative interface. <warning><para>Allowing
 * access to this interface is a security risk! Please see the &IcePack;
 * documentation for further information.</para></warning>
 *
 **/
class Admin
{
    /**
     *
     * All server descriptions.
     *
     **/
    ServerDescriptions _serverDescriptions;

    /**
     *
     * Add a server and objects implemented by that server to &IcePack;.
     *
     * @param description The server's description.
     *
     * @see remove
     *
     **/
    void add(ServerDescription description);

    /**
     *
     * Remove a server and objects implemented by that server from &IcePack;.
     *
     * @param identity Must match the identity of the
     * [ServerDescription::object].
     *
     * @see add
     *
     **/
    void remove(Ice::Identity identity);

    /**
     *
     * Find a server and objects implemented by that server from &IcePack;.
     *
     * @param identity Must match the identity of the
     * [ServerDescription::object].
     *
     * @return The server description, or null if no description was found.
     *
     * @see add
     *
     **/
    ServerDescription find(Ice::Identity identity);

    /**
     *
     * Get all server descriptions in &IcePack;.
     *
     * @return The descriptions of all servers.
     *
     **/
    ServerDescriptions getAll();

    /**
     *
     * Shut down &IcePack;.
     *
     **/
    void shutdown();
};

};

#endif
