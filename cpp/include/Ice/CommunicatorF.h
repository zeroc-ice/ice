// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Generated from file `Communicator.ice'

#ifndef __CommunicatorF_h__
#define __CommunicatorF_h__

#include <Ice/LocalObjectF.h>

namespace Ice
{

class ObjectAdapter;

class ValueFactory;

class Properties;

class Logger;

class Pickler;

class Communicator;

}

namespace __Ice
{

void incRef(::Ice::ObjectAdapter*);
void decRef(::Ice::ObjectAdapter*);

void incRef(::Ice::ValueFactory*);
void decRef(::Ice::ValueFactory*);

void incRef(::Ice::Properties*);
void decRef(::Ice::Properties*);

void incRef(::Ice::Logger*);
void decRef(::Ice::Logger*);

void incRef(::Ice::Pickler*);
void decRef(::Ice::Pickler*);

void incRef(::Ice::Communicator*);
void decRef(::Ice::Communicator*);

}

namespace Ice
{

typedef ::__Ice::Handle< ::Ice::ObjectAdapter> ObjectAdapter_ptr;

typedef ::__Ice::Handle< ::Ice::ValueFactory> ValueFactory_ptr;

typedef ::__Ice::Handle< ::Ice::Properties> Properties_ptr;

typedef ::__Ice::Handle< ::Ice::Logger> Logger_ptr;

typedef ::__Ice::Handle< ::Ice::Pickler> Pickler_ptr;

typedef ::__Ice::Handle< ::Ice::Communicator> Communicator_ptr;

}

#endif
