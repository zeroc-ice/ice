// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SERVANT_LOCATOR_ICE
#define ICE_SERVANT_LOCATOR_ICE

#include <Ice/ObjectAdapterF.ice>
#include <Ice/Current.ice>

module Ice
{

/**
 *
 * The Servant Locator, which is called by the object adapter to
 * locate a Servant which is not found in its Active Servant Map.
 *
 * @see ObjectAdapter
 * @see ObjectAdapter::addServantLocator
 * @see ObjectAdapter::removeServantLocator
 * @see ObjectAdapter::findServantLocator
 *
 **/
local interface ServantLocator
{
    /**
     *
     * Called by the object adapter before a request is made, in case
     * a Servant cannot be found in the object adapter's Active
     * Servant Map. Note that the object adapter does not
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
     * @param adapter The object adapter that calls the Servant
     * Locator.
     *
     * @param current Information about the current operation call to
     * locate a Servant for.
     *
     * @param cookie A "cookie", which will be passed to [finished].
     *
     * @return The located Servant, or null if no suitable Servant has
     * been found.
     *
     * @see ObjectAdapter
     * @see Current
     * @see finished
     *
     **/
    Object locate(ObjectAdapter adapter, Current current, out LocalObject cookie);

    /**
     *
     * Called by the object adapter after a request has been
     * made. This operation is only called if [locate] was called
     * prior to the request and returned a non-null servant. This
     * operation can be used for cleanup purposes after a request.
     *
     * @param adapter The object adapter that calls the Servant Locator.
     *
     * @param current Information about the current operation call for
     * which a servant was lcoated by [locate].
     *
     * @param servant The Servant that was returned by [locate].
     *
     * @param cookie The cookie that was returned by [locate].
     *
     * @see ObjectAdapter
     * @see Current
     * @see locate
     *
     **/
    void finished(ObjectAdapter adapter, Current current, Object servant, LocalObject cookie);

    /**
     *
     * Called when the object adapter in which this Servant Locator is
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
