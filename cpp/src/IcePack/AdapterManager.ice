// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ADAPTER_MANAGER_ICE
#define ICE_PACK_ADAPTER_MANAGER_ICE

#include <IcePack/Admin.ice>

module IcePack
{

/**
 *
 * Forward declaration.
 *
 */
class Server;

/**
 *
 * This exception is raised if the activation of an adapter exceed the
 * time specified by the <literal>IcePack.Activation.WaitTime</literal>
 * configuration variable.
 *
 **/
exception AdapterActivationTimeoutException 
{ 
    string message; 
};

struct AdapterDescription
{
    /**
     *
     * Adapter name.
     *
     **/
    string name;

    /**
     *
     * Server proxy to which this adapter is associated to. If null,
     * no automatic activation of the server will be performed.
     *
     **/
    Server* server;
};

class Adapter
{
    /**
     *
     * Adapter description.
     *
     * @return The adapter description.
     *
     **/
    AdapterDescription getAdapterDescription();

    /**
     *
     * Get a proxy for this adapter.
     *
     * @param activate If true and if the adapter is registered with a
     * server, the server will be started if the adapter status is
     * inactive. If false, no activation will be performed and the
     * last known direct proxy will be returned.
     *
     * @return A direct proxy for the adapter.
     *
     * @throws ActivationTimeoutException Raised if the time needed to
     * activate the adapter exceeded the time specified by the
     * <literal>IcePack.Activation.WaitTime</literal> configuration
     * variable.
     *
     **/
    Object* getDirectProxy(bool activate)
	throws AdapterActivationTimeoutException;

    /**
     *
     * Set the active proxy created from the adapter.
     *
     **/
    void setDirectProxy(Object* proxy);

    /**
     *
     * Mark the adapter as active.
     *
     **/
    void markAsActive();

    /**
     *
     * Mark the adapter as inactive.
     *
     **/
    void markAsInactive();

    /**
     * 
     * The description of this objet adapter. 
     *
     */
    AdapterDescription description;

    /**
     *
     * A direct proxy created from the adapter.
     *
     */
    Object* proxy;
};

/**
 *
 * A sequence of adapter proxies.
 *
 **/
sequence<Adapter*> Adapters;

class AdapterManager
{
    /**
     *
     * Create an adapter.
     *
     **/
    Adapter* create(AdapterDescription description)
	throws AdapterExistsException;

    /**
     *
     * Find an adapter and return its proxy.
     *
     * @param name Name of the adapter.
     *
     * @return Adapter proxy.
     *
     **/
     Adapter* findByName(string name);

    /**
     *
     * Remove an adapter.
     *
     **/
    void remove(string name)
	throws AdapterNotExistException;
    
    /**
     *
     * Get all adapter names.
     *
     **/
    AdapterNames getAll();
};

};

#endif
