// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Object.h>

class BlobjectI : public Ice::Blobject
{
public:

    virtual bool ice_invoke(ICE_IN(std::vector<Ice::Byte>), std::vector<Ice::Byte>&, const Ice::Current&);
};

class BlobjectArrayI : public Ice::BlobjectArray
{
public:

    virtual bool ice_invoke(ICE_IN(std::pair<const Ice::Byte*, const Ice::Byte*>), std::vector<Ice::Byte>&,
                            const Ice::Current&);
};

#ifdef ICE_CPP11_MAPPING
class BlobjectAsyncI : public Ice::BlobjectAsync
{
public:

    virtual void ice_invokeAsync(std::vector<Ice::Byte>,
                                 std::function<void(bool, const std::vector<Ice::Byte>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);
};

class BlobjectArrayAsyncI : public Ice::BlobjectArrayAsync
{
public:

    virtual void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*>,
                                 std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);
};
#else
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

#endif
