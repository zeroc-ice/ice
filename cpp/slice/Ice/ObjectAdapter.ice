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

local interface ServantLocator;

/**
 *
 * The Object Adapter, which is responsible for receiving requests
 * from Endpoints, and for mapping between Servant, Identities, and
 * Proxies.
 *
 * @see Communicator
 * @see ServantLocator
 *
 **/
local interface ObjectAdapter
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
     * Adapter can be reactivated with the [activate] operation.
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
     * requests through its Endpoints. Object Adapter's that have been
     * deactivated must not be reactivated again, i.e., the
     * deactivation is permanent and [activate] or [hold] must not be
     * called after calling [deactivate]. Attempting to do so results
     * in an [ObjectAdapterDeactivatedException] being thrown. Calls
     * to [deactivate] on an already deactivated Object Adapter are
     * ignored.
     *
     * @see activate
     * @see hold
     * @see Communicator::shutdown
     *
     **/
    void deactivate();

    /**
     *
     * Add a Servant to this Object Adapter's Active Servant Map. Note
     * that one Servant can implement several Ice Objects by
     * registering the Servant with multiple identities.
     *
     * @param servant The Servant to add.
     *
     * @param identity The identity of the Ice Object that is
     * implemented by the Servant.
     *
     * @return A Proxy that matches the given identity and this Object
     * Adapter.
     *
     * @see addTemporary
     * @see remove
     *
     **/
    Object* add(Object servant, string identity);

    /**
     *
     * Add a temporary Servant to this Object Adapter's Active Servant
     * Map. "Temporary" means that the Ice Object implemented by the
     * Servant does not have a fixed identity. Instead, a temporary
     * identity is assigned by the Object Adapter. Such temporary
     * identity is only valid for the lifetime of this Object Adapter,
     * or until the Servant is removed with [remove].
     *
     * @param servant The Servant to add.
     *
     * @return A Proxy that matches the temporary identity and this
     * Object Adapter.
     *
     * @see add
     * @see remove
     *
     **/
    Object* addTemporary(Object servant);

    /**
     *
     * Remove a Servant from the Object Adapter's Active Servant Map.
     *
     * @param identity The identity of the Ice Object that is
     * implemented by the Servant. If the Servant implements multiple
     * Ice Objects, [remove] has to be called for all such Ice
     * Objects.
     *
     * @see add
     * @see addTemporary
     *
     **/
    void remove(string identity);

    /**
     *
     * Add a Servant Locator to this Object Adapter. If a locator has
     * already been installed for the given prefix, the current
     * locator for this prefix is replaced by the new one. To dispatch
     * operation calls on Servants, the Object Adapter tries to find a
     * Servant for a given Ice Object identity in the following order:
     *
     * <orderedlist>
     *
     * <listitem><para>The Object Adapter tries to find a Servant for
     * the identity in the Active Servant Map.</para></listitem>
     *
     * <listitem><para>If no Servant has been found in the Active
     * Servant Map, and the identity has the format
     * "<replaceable>prefix</replaceable><literal>#</literal>", the
     * Object Adapter tries to find a locator for this prefix. If a
     * locator is found, the Object Adapter tries to find a Servant
     * using this locator.</para></listitem>
     *
     * <listitem><para>If no Servant has been found by any of the
     * preceding steps, the Object Adapter tries to find a locator
     * with an empty prefix, regardless of the format of the identity,
     * i.e., even if the identity contains a
     * '<literal>#</literal>'. If a locator is found, the Object
     * Adapter tries to find a Servant using this
     * locator</para></listitem>
     *
     * <listitem><para>If no Servant has been found with any of the
     * preceding steps, the Object Adapter gives up and the caller
     * will receive an [ObjectNotExistException].</para></listitem>
     *
     * </orderedlist>
     *
     * <note><para>Only one locator for an empty prefix can be
     * installed.</para></note>
     *
     * @param locator The locator to add.
     *
     * @param prefix The Ice Object identity prefix for which the
     * Servant Locator can locate Servants.
     *
     * @see removeServantLocator
     * @see findServantLocator
     * @see ServantLocator
     *
     **/
    void addServantLocator(ServantLocator locator, string prefix);

    /**
     *
     * Remove a Servant locator from this Object Adapter. This
     * operation does nothing if no locator for the given prefix has
     * been installed.
     *
     * @param prefix The Ice Object identity prefix for which the
     * Servant Locator can locate Servants.
     *
     * @see addServantLocator
     * @see findServantLocator
     * @see ServantLocator
     *
     **/
    void removeServantLocator(string prefix);

    /**
     *
     * Find a Servant Locator installed with this Object Adapter.
     *
     * @param prefix The Ice Object identity prefix for which the
     * Servant Locator can locate Servants.
     *
     * @return The Servant Locator, or null if no Servant Locator was
     * found for the given prefix.
     *
     * @see addServantLocator
     * @see removeServantLocator
     * @see ServantLocator
     *
     **/
    ServantLocator findServantLocator(string prefix);

    /**
     *
     * Look up a Servant in this Object Adapter's Active Servant Map
     * by the identity of the Ice Object it implements.
     *
     * <note><para>This operation does only try to lookup a Servant in
     * the Active Servant Map. It does not attempt to find a Servant
     * by using any installed [ServantLocator].</para></note>
     *
     * @param identity The identity of the Ice Object for which the
     * Servant should be returned.
     *
     * @return The Servant that implements the Ice Object with the
     * given identity, or null if no such Servant has been found.
     *
     * see proxyToServant
     *
     **/
    Object identityToServant(string identity);

    /**
     *
     * Look up a Servant in this Object Adapter's Active Servant Map,
     * given a Proxy.
     *
     * <note><para>This operation does only try to lookup a Servant in
     * the Active Servant Map. It does not attempt to find a Servant
     * via any installed [ServantLocator]s.</para></note>
     *
     * @param Proxy The proxy for which the Servant should be returned.
     *
     * @return The Servant that matches the Proxy, or null if no such
     * Servant has been found.
     *
     * see identityToServant
     *
     **/
    Object proxyToServant(Object* proxy);

    /**
     *
     * Create a Proxy that matches this Object Adapter and the given
     * identity.
     *
     * @param identity The identity for which a Proxy is to be created.
     *
     * @return A Proxy that matches the given identity and this Object
     * Adapter.
     *
     **/
    Object* createProxy(string identity);
};

/**
 *
 * The Servant Locator, which is called by the Object Adapter to
 * locate Servants, which it doesn't find in its Active Servant
 * Map. The Servant Locator must be set with
 * [ObjectAdapter::setServantLocator] operation.
 *
 * @see ObjectAdapter
 *
 **/
local interface ServantLocator
{
    /**
     *
     * Called by the Object Adapter before a request is made, in case
     * a Servant cannot be found in the Object Adapter's Active
     * Servant Map. Note that the Object Adapter does not
     * automatically insert the returned Servant into it's Active
     * Servant Map. This must be done by the Servant Locator's
     * implementation, if this is desired.
     *
     * <important><para>If you call [locate] from our own code, you
     * must also call [finished] when you are done with using the
     * Servant, provided that a non-null Servant was
     * returned. Otherwise you will get undefined behavior if you use
     * Servant Locators such as the
     * [Freeze::Evictor].</para></important>
     *
     * @param adapter The Object Adapter that calls the Servant
     * Locator.
     *
     * @param identity The identity of the Ice Object to locate a
     * Servant for.
     *
     * @param operation The operation the Object Adapter is about to
     * call.
     *
     * @param cookie A "cookie", which will be passed to [finished].
     *
     * @return The located Servant, or null if no suitable Servant has
     * been found.
     *
     * @see ObjectAdapter
     * @see finished
     *
     **/
    Object locate(ObjectAdapter adapter, string identity, string operation; LocalObject cookie);

    /**
     *
     * Called by the Object Adapter after a request has been
     * made. This operation is only called if [locate] was called
     * prior to the request and returned a non-null servant. This
     * operation can be used for cleanup purposes after a request.
     *
     * @param adapter The Object Adapter that calls the Servant Locator.
     *
     * @param identity The identity of the Ice Object for which a
     * Servant was located by [locate].
     *
     * @param servant The Servant that was returned by [locate].
     *
     * @param operation The operation the Object Adapter just called.
     *
     * @param cookie The cookie that was returned by [locate].
     *
     * @see ObjectAdapter
     * @see locate
     *
     **/
    void finished(ObjectAdapter adapter, string identity, Object servant, string operation, LocalObject cookie);

    /**
     *
     * Called when the Object Adapter this Servant Locator is
     * installed with is deactivated.
     *
     * @see ObjectAdapter::deactivate
     * @see Communicator::shutdown
     * @see Communicator::destroy
     *
     **/
    void deactivate();
};

};

#endif
