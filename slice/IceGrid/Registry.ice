//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:IceGrid/Config.h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICEGRID_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:IceGrid"]]

#include <IceGrid/Exception.ice>
#include <IceGrid/Session.ice>
#include <IceGrid/Admin.ice>
#include <Ice/Locator.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEGRID"]
module IceGrid
{

/**
 *
 * Determines which load sampling interval to use.
 *
 **/
enum LoadSample
{
    /**
     * Sample every minute.
     **/
    LoadSample1,

    /**
     * Sample every five minutes.
     **/
    LoadSample5,

    /**
     * Sample every fifteen minutes.
     **/
    LoadSample15
}

/**
 *
 * The IceGrid query interface. This interface is accessible to
 * Ice clients who wish to look up well-known objects.
 *
 **/
interface Query
{
    /**
     *
     * Find a well-known object by identity.
     *
     * @param id The identity.
     *
     * @return The proxy or null if no such object has been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* findObjectById(Ice::Identity id);

    /**
     *
     * Find a well-known object by type. If there are several objects
     * registered for the given type, the object is randomly
     * selected.
     *
     * @param type The object type.
     *
     * @return The proxy or null, if no such object has been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* findObjectByType(string type);

    /**
     *
     * Find a well-known object by type on the least-loaded node. If
     * the registry does not know which node hosts the object
     * (for example, because the object was registered with a direct proxy), the
     * registry assumes the object is hosted on a node that has a load
     * average of 1.0.
     *
     * @param type The object type.
     *
     * @param sample The sampling interval.
     *
     * @return The proxy or null, if no such object has been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* findObjectByTypeOnLeastLoadedNode(string type, LoadSample sample);

    /**
     *
     * Find all the well-known objects with the given type.
     *
     * @param type The object type.
     *
     * @return The proxies or an empty sequence, if no such objects
     * have been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::ObjectProxySeq findAllObjectsByType(string type);

    /**
     *
     * Find all the object replicas associated with the given
     * proxy. If the given proxy is not an indirect proxy from a
     * replica group, an empty sequence is returned.
     *
     * @param proxy The object proxy.
     *
     * @return The proxies of each object replica or an empty sequence,
     * if the given proxy is not from a replica group.
     *
     **/
    ["cpp:const"] idempotent Ice::ObjectProxySeq findAllReplicas(Object* proxy);
}

/**
 *
 * The IceGrid registry allows clients create sessions
 * directly with the registry.
 *
 * @see Session
 * @see AdminSession
 *
 **/
interface Registry
{
    /**
     *
     * Create a client session.
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    Session* createSession(string userId, string password)
        throws PermissionDeniedException;

    /**
     *
     * Create an administrative session.
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    AdminSession* createAdminSession(string userId, string password)
        throws PermissionDeniedException;

    /**
     *
     * Create a client session from a secure connection.
     *
     * @return A proxy for the newly created session.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    Session* createSessionFromSecureConnection()
        throws PermissionDeniedException;

    /**
     *
     * Create an administrative session from a secure connection.
     *
     * @return A proxy for the newly created session.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    AdminSession* createAdminSessionFromSecureConnection()
        throws PermissionDeniedException;

    /**
     *
     * Get the session timeout. If a client or administrative client
     * doesn't call the session keepAlive method in the time interval
     * defined by this timeout, IceGrid might reap the session.
     *
     * @see Session#keepAlive
     * @see AdminSession#keepAlive
     *
     * @return The timeout (in seconds).
     *
     **/
    ["nonmutating", "cpp:const"] idempotent int getSessionTimeout();

    /**
     *
     * Get the value of the ACM timeout. Clients supporting ACM
     * connection heartbeats can enable them instead of explicitly
     * sending keep alives requests.
     *
     * NOTE: This method is only available since Ice 3.6.
     *
     * @return The timeout (in seconds).
     *
     **/
    ["nonmutating", "cpp:const"] idempotent int getACMTimeout();
}

/**
 *
 * The IceGrid locator interface provides access to the {@link Query}
 * and {@link Registry} object of the IceGrid registry.
 *
 * @see Query
 * @see Registry
 *
 **/
interface Locator extends Ice::Locator
{
    /**
     *
     * Get the proxy of the registry object hosted by this IceGrid
     * registry.
     *
     * @return The proxy of the registry object.
     *
     **/
    ["cpp:const"] idempotent Registry* getLocalRegistry();

    /**
     *
     * Get the proxy of the query object hosted by this IceGrid
     * registry.
     *
     * @return The proxy of the query object.
     *
     **/
    ["cpp:const"] idempotent Query* getLocalQuery();
}

}
