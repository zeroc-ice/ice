// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
class ImplicitContextI;
ICE_DEFINE_PTR(ImplicitContextIPtr,ImplicitContextI);

class ImplicitContextI : public ImplicitContext
{
public:

    static ImplicitContextIPtr create(const std::string&);

#ifdef _WIN32
    static void cleanupThread();
#endif

    //
    // Marshals the underlying context plus the given context
    // (entries in the given context overwrite entries in
    // the underlying context)
    //
    virtual void write(const Context&, ::Ice::OutputStream*) const = 0;

    //
    // Combines the underlying context plus the given context
    // (entries in the given context overwrite entries in
    // the underlying context)
    //
    virtual void combine(const Context&, Context&) const = 0;

};

ICE_DEFINE_PTR(ImplicitContextIPtr, ImplicitContextI);

}
#endif
