// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Generated from file `Logger.ice'

#ifndef __Logger_h__
#define __Logger_h__

#include <Ice/LocalObject.h>

namespace Ice
{

class Logger;

}

namespace __Ice
{

void ICE_API incRef(::Ice::Logger*);
void ICE_API decRef(::Ice::Logger*);

}

namespace Ice
{

typedef ::__Ice::Handle< ::Ice::Logger> Logger_ptr;

}

namespace Ice
{

class ICE_API Logger : public ::Ice::LocalObject
{
public: 

    virtual void trace(const ::std::string&, const ::std::string&) = 0;

    virtual void warning(const ::std::string&) = 0;

    virtual void error(const ::std::string&) = 0;

    virtual void __write(::__Ice::Stream*);
    virtual void __read(::__Ice::Stream*);
};

}

#endif
