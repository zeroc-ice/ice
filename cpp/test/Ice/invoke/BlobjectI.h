// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Object.h>

class BlobjectI : public Ice::Blobject
{
public:

    virtual bool ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);
};

class BlobjectArrayI : public Ice::BlobjectArray
{
public:

    virtual bool ice_invoke(const std::pair<const Ice::Byte*, const Ice::Byte*>&, std::vector<Ice::Byte>&,
                            const Ice::Current&);
};


class BlobjectAsyncI : public Ice::BlobjectAsync
{
public:

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const std::vector<Ice::Byte>&,
                                  const Ice::Current&);
};

class BlobjectArrayAsyncI : public Ice::BlobjectArrayAsync
{
public:

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, 
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
};

#endif
