// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef HELLO_ICE
#define HELLO_ICE

class Hello
{
    /**
     *
     * Display message on the standard output.
     *
     **/
    nonmutating void sayHello();

    /**
     *
     * Destroy the object.
     *
     **/
    void destroy();

    /**
     *
     * The [Hello] object name.
     *
     **/
    string name;
};

exception NameExistsException
{
};

exception NameNotExistException
{
};

interface HelloFactory
{
    /**
     *
     * Create a [Hello] object.
     *
     * @param name The name of the [Hello] object.
     *
     * @return A [Hello] object
     *
     * @throws NameExistsException Raised if a [Hello] object with
     * the name already exists.
     *
     **/
    Hello* create(string name)
	throws NameExistsException;
    
    /**
     *
     * Find a [Hello] object.
     *
     * @param name The name of the [Hello] object.
     *
     * @return The [Hello] object.
     * 
     * @throws NameNotExistException Raised if the [Hello] object
     * can't be found.
     *
     **/
    nonmutating Hello* find(string name)
	throws NameNotExistException;
};

#endif
