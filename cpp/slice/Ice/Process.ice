// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PROCESS_ICE
#define ICE_PROCESS_ICE

module Ice
{

/**
 *
 * An administrative interface for process management. Managed servers must
 * implement this interface and invoke [ObjectAdapter::setProcess] to register
 * the process proxy.
 *
 * <note><para> A servant implementing this interface is a potential target
 * for denial-of-service attacks, therefore proper security precautions
 * should be taken. For example, the servant can use a UUID to make its
 * identity harder to guess, and be registered in an object adapter with
 * a secured endpoint.</para></note>
 *
 **/
interface Process
{
    /**
     *
     * Initiate a graceful shutdown.
     *
     * @see Communicator::shutdown
     *
     **/
    idempotent void shutdown();
};

};

#endif
