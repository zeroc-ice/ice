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

void ICE_API incRef(::Ice::ObjectAdapter*);
void ICE_API decRef(::Ice::ObjectAdapter*);

void ICE_API incRef(::Ice::ValueFactory*);
void ICE_API decRef(::Ice::ValueFactory*);

void ICE_API incRef(::Ice::Properties*);
void ICE_API decRef(::Ice::Properties*);

void ICE_API incRef(::Ice::Logger*);
void ICE_API decRef(::Ice::Logger*);

void ICE_API incRef(::Ice::Pickler*);
void ICE_API decRef(::Ice::Pickler*);

void ICE_API incRef(::Ice::Communicator*);
void ICE_API decRef(::Ice::Communicator*);

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
