// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_IDENTITY_ICE
#define ICE_IDENTITY_ICE

module Ice
{

/**
 *
 * The identity of an &Ice; object. An empty [name] denotes a null
 * object.
 *
 **/
struct Identity
{
    /**
     *
     * The name of the &Ice; object.
     *
     **/
    string name;

    /**
     *
     * The &Ice; object category.
     *
     * @see ServantLocator
     * @see ObjectAdapter::addServantLocator
     *
     **/
    string category;
};

};

#endif
