// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER_SESSION_ICE
#define GLACIER_SESSION_ICE

module Glacier
{

/**
 *
 * A session object, which is tied to the lifecycle of a [Router].
 *
 * @see Router
 * @see SessionManager
 *
 **/
interface Session
{
    /**
     *
     * Destroy the session. This is called automatically when the
     * [Router] is destroyed.
     *
     **/
    void destroy();
};

};

#endif
