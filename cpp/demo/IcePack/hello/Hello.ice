// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_ICE
#define HELLO_ICE

module Demo
{

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

};

#endif
