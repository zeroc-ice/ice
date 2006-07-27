// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BOX_ICE_BOX_ICE
#define ICE_BOX_ICE_BOX_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/CommunicatorF.ice>
#include <Ice/PropertiesF.ice>
#include <Ice/SliceChecksumDict.ice>

/**
 *
 * &IceBox; is an application server specifically for &Ice;
 * applications. &IceBox; can easily run and administer Ice services
 * that are dynamically loaded as a DLL, shared library, or Java
 * class.
 *
 **/
module IceBox
{
    
/**
 *
 * Indicates a failure occurred. For example, if a service encounters
 * an error during initialization, or if the service manager is unable
 * to load a service executable.
 *
 **/
local exception FailureException
{
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};

/**
 *
 * An application service managed by a [ServiceManager].
 *
 **/
local interface Service
{
    /**
     *
     * Start the service. The given communicator is created by the
     * [ServiceManager] for use by the service. This communicator may
     * also be used by other services, depending on the service
     * configuration.
     *
     * <note><para>The [ServiceManager] owns this communicator, and is
     * responsible for destroying it.</para></note>
     *
     * @param name The service's name, as determined by the
     * configuration.
     *
     * @param communicator A communicator for use by the service.
     *
     * @param args The service arguments that were not converted into
     * properties.
     *
     * @throws FailureException Raised if [start] failed.
     *
     **/
    void start(string name, Ice::Communicator communicator, Ice::StringSeq args);

    /**
     *
     * Stop the service.
     *
     **/
    void stop();
};

/**
 *
 * Administers a set of [Service] instances.
 *
 * @see Service
 *
 **/
interface ServiceManager
{
    /**
     *
     * Returns the checksums for the IceBox Slice definitions.
     *
     * @return A dictionary mapping Slice type ids to their checksums.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::SliceChecksumDict getSliceChecksums();

    /**
     *
     * Shutdown all services. This will cause [Service::stop] to be
     * invoked on all configured services.
     *
     **/
    void shutdown();
};

};

#endif
