// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SERVANT_LOCATOR_ICE
#define ICE_SERVANT_LOCATOR_ICE

#include <Ice/ObjectAdapterF.ice>

module Ice
{

/**
 *
 * The Servant Locator, which is called by the Object Adapter to
 * locate a Servant which is not found in its Active Servant
 * Map. The Servant Locator must be set with
 * [ObjectAdapter::addServantLocator] operation.
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
     * <important><para>If you call [locate] from your own code, you
     * must also call [finished] when you have finished using the
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
     * @param operation The operation the Object Adapter just called.
     *
     * @param servant The Servant that was returned by [locate].
     *
     * @param cookie The cookie that was returned by [locate].
     *
     * @see ObjectAdapter
     * @see locate
     *
     **/
    void finished(ObjectAdapter adapter, string identity, string operation, Object servant, LocalObject cookie);

    /**
     *
     * Called when the Object Adapter in which this Servant Locator is
     * installed is deactivated.
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
