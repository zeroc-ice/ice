// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_ICE
#define ICE_OBJECT_ADAPTER_ICE

#include <Ice/CommunicatorF.ice>

module Ice
{

local class ObjectLocator;

/**
 *
 * The Object Adapter, which is responsible for receiving requests
 * from Endpoints, and for mapping between Objects, Identities, and
 * Proxies. Object Adapters are created with the Communicator's
 * <literal>createObjectAdapter</literal> and
 * <literal>createObjectAdapterWithEndpoints</literal> operations.
 *
 * @see Communicator
 * @see ObjectLocator
 *
 **/
local class ObjectAdapter
{
    /**
     *
     * Get the name of this Object Adapter. The name is mainly used
     * for configuration purposes with Properties.
     *
     * @return The name of this Object Adapter.
     *
     * @see Properties
     *
     **/
    string getName();

    /**
     *
     * Get the Communicator this Object Adapter belongs to.
     *
     * @return This Object Adapter's Communicator.
     *
     * @see Communicator
     *
     **/
    Communicator getCommunicator();

    /**
     *
     * Activate all Endpoints that belong to this Object
     * Adapter. After activation, the Object Adapter can dispatch
     * requests received through its Endpoints.
     *
     * @see hold
     * @see deactivate
     *
     **/
    void activate();

    /**
     *
     * Temporarily hold receiving and dispatching requests. The Object
     * Adapter can be reactivated with the <literal>activate</literal>
     * operation.
     *
     * @see activate
     * @see deactivate
     *
     **/
    void hold();

    /**
     *
     * Deactivate all Endpoints that belong to this Object
     * Adapter. After deactivation, the Object Adapter stops receiving
     * requests through its Endpoints. Object Adapter's that have
     * been deactivated must not be reactivated again, i.e., the
     * deactivation is permanent and <literal>activate</literal> or
     * <literal>hold</literal> must not be called after calling
     * <literal>deactivate</literal>.
     *
     * @see activate
     * @see hold
     *
     **/
    void deactivate();

    /**
     *
     * Add a new Object to this Object Adapter's Active Object Map.
     *
     * @param object The Object to add.
     *
     * @param identity The Object's identity.
     *
     * @see addTemporary
     * @see remove
     *
     **/
    void add(Object object, string identity);

    /**
     *
     * Add a new temporary Object to this Object Adapter's Active
     * Object Map. "Temporary" means that the Object does not have a
     * fixed identity. Instead, a temporary identity is assigned by
     * the Object Adapter. Such temporary identity is only valid for
     * the lifetime of this Object Adapter, or until the Object is
     * removed with <literal>remove</literal>.
     *
     * @param object The Object to add.
     *
     * @param identity The Object's identity.
     *
     * @see addTemporary
     * @see remove
     *
     **/
    void addTemporary(Object object);

    /**
     *
     * Remove an Object from the Object Adapter's Active Object Map.
     *
     * @param identity The Object's identity that is to be removed.
     *
     * @see add
     * @see addTemporary
     *
     **/
    void remove(string identity);

    /**
     *
     * Set an Object Locator for this Object Adapter.
     *
     * @param locator The locator to set.     
     *
     * @see ObjectLocator
     * @see getObjectLocator
     *
     **/
    void setObjectLocator(ObjectLocator locator);

    /**
     *
     * Get the Object Locator for this Object Adapter.
     *
     * @return The locator for this Object Adapter. If no locator is
     * set, null is returned.
     *
     * @see ObjectLocator
     * @see setObjectLocator
     *
     **/
    ObjectLocator getObjectLocator();

    /**
     *
     * Look up an Object in this Object Adapter's Active Object Map by
     * its identity.
     *
     * @param identity The identity for which the Object should be
     * returned.
     *
     * @return The Object that matches the identity.
     *
     * see proxyToObject
     * see objectToIdentity
     *
     **/
    Object identityToObject(string identity);

    /**
     *
     * Loop up the identity for an Object in this Object Adapter's
     * Active Object Map.
     *
     * @param object The Object for which the identity should be
     * returned.
     *
     * @return The identity that matches this Object.
     *
     * see identityToObject
     * see identityToProxy
     *
     **/
    string objectToIdentity(Object object);

    /**
     *
     * Look up an Object in this Object Adapter's Active Object Map
     * given a Proxy.
     *
     * @param Proxy A proxy for which the Object should be returned.
     *
     * @return The Object that matches the Proxy.
     *
     * see objectToProxy
     * see identityToObject
     *
     **/
    Object proxyToObject(Object* proxy);

    /**
     *
     * Create a Proxy for an Object in this Object Adapter's Active
     * Object Map.
     *
     * @param object The Object for which a Proxy is to be created.
     *
     * @return A Proxy for the Object.
     *
     * see proxyToObject
     * see objectToIdentity
     *
     **/
    Object* objectToProxy(Object object);

    /**
     *
     * Create a Proxy that matches this Object Adapter and the given
     * identity.
     *
     * @param identity The identity for which a Proxy is to be created.
     *
     * @return A Proxy that matches the identity and this Object
     * Adapter.
     *
     * see identityToObject
     * see proxyToIdentity
     *
     **/
    Object* identityToProxy(string identity);

    /**
     *
     * Extract the identity from a Proxy.
     *
     * @param proxy The Proxy from which the identity is to be extracted.
     *
     * @return The Proxy's identity.
     *
     * see proxyToObject
     * see identityToProxy
     *
     **/
    string proxyToIdentity(Object* proxy);
};

/**
 *
 * The Object Locator, which is called by the Object Adapter to locate
 * objects, which it doesn't find in its Active Object Map. The
 * locator must be set with the Object Adapter's
 * <literal>setObjectLocator</literal> operation.
 *
 * @see ObjectAdapter
 * @see ObjectAdapter::setObjectLocator
 * @see ObjectAdapter::getObjectLocator
 *
 **/
local class ObjectLocator
{
    /**
     *
     * Called by the Object Adapter before a request, if an object
     * cannot be found in the Object Adapter's Active Object Map. Note
     * that the Object Adapter does not automatically insert the
     * returned object into it's Active Object Map. This must be done
     * by the locator if desired.
     *
     * @param adapter The Object Adapter that calls the locator.
     *
     * @param identity The identity of the object to locate.
     *
     * @param cookie A "cookie", which is returned to
     * <literal>finished</literal>.
     *
     * @return The located object, or null if no object has been
     * found.
     *
     * @see ObjectAdapter
     * @see finished
     *
     **/
    Object locate(ObjectAdapter adapter, string identity; Object cookie);

    /**
     *
     * Called by the Object Adapter after a request, provided that
     * <literal>locate</literal> did not return null. This operation
     * can be used for cleanup after a request.
     *
     * @param adapter The Object Adapter that calls the locator.
     *
     * @param identity The identity of the object that was located by
     * <literal>locate</literal>.
     *
     * @param object The object that was returned by
     * <literal>locate</literal>.
     *
     * @param cookie The cookie that was returned by
     * <literal>locate</literal>.
     *
     * @see ObjectAdapter
     * @see locate
     *
     **/
    void finished(ObjectAdapter adapter, string identity, Object object, Object cookie);
};

};

#endif
