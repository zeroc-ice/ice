// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_IMPLICIT_CONTEXT_I_H
#define ICE_IMPLICIT_CONTEXT_I_H

#include <Ice/ImplicitContext.h>

namespace Ice
{

//
// The base class for all ImplicitContext implementations
//

class ImplicitContextI : public ImplicitContext
{
public:
    
    static ImplicitContextI* create(const std::string&);

#ifdef _WIN32
    static void cleanupThread();
#endif

    //
    // Marshals the underlying context plus the given context
    // (entries in the given context overwrite entries in
    // the underlying context)
    //
    virtual void write(const Context&, ::IceInternal::BasicStream*) const = 0;

    //
    // Combines the underlying context plus the given context
    // (entries in the given context overwrite entries in
    // the underlying context)
    //
    virtual void combine(const Context&, Context&) const = 0;

};

typedef IceInternal::Handle<ImplicitContextI> ImplicitContextIPtr;

}
#endif
