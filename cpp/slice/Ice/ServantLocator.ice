// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
 * The servant locator, which is called by the object adapter to
 * locate a servant that is not found in its active servant map.
 *
 * @see ObjectAdapter
 * @see ObjectAdapter::addServantLocator
 * @see ObjectAdapter::findServantLocator
 *
 **/
local interface ServantLocator
{
    /**
     *
     * Called by the object adapter before a request is made when a
     * servant cannot be found in the object adapter's active servant
     * map. Note that the object adapter does not automatically insert
     * the returned servant into its active servant map. This must be
     * done by the servant locator implementation, if this is desired.
     *
     * <important><para>If you call [locate] from your own code, you
     * must also call [finished] when you have finished using the
     * servant, provided that a non-null servant was
     * returned. Otherwise you will get undefined behavior if you use
     * Servant Locators such as the
     * [Freeze::Evictor].</para></important>
     *
     * @param curr Information about the current operation for which
     * a servant is required.
     *
     * @param cookie A "cookie" that will be passed to [finished].
     *
     * @return The located servant, or null if no suitable servant has
     * been found.
     *
     * @see ObjectAdapter
     * @see Current
     * @see finished
     *
     **/
    Object locate(Current curr, out LocalObject cookie);

    /**
     *
     * Called by the object adapter after a request has been
     * made. This operation is only called if [locate] was called
     * prior to the request and returned a non-null servant. This
     * operation can be used for cleanup purposes after a request.
     *
     * @param curr Information about the current operation call for
     * which a servant was located by [locate].
     *
     * @param servant The servant that was returned by [locate].
     *
     * @param cookie The cookie that was returned by [locate].
     *
     * @see ObjectAdapter
     * @see Current
     * @see locate
     *
     **/
    void finished(Current curr, Object servant, LocalObject cookie);

    /**
     *
     * Called when the object adapter in which this servant locator is
     * installed is deactivated.
     *
     * @param category Indicates for which category the servant locator
     * is being deactivated.
     *
     * @see ObjectAdapter::deactivate
     * @see Communicator::shutdown
     * @see Communicator::destroy
     *
     **/
    void deactivate(string category);
};

};

#endif
