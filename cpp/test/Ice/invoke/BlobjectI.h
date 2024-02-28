//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Object.h>

class BlobjectI : public Ice::Blobject
{
public:

    virtual bool ice_invoke(std::vector<std::uint8_t>, std::vector<std::uint8_t>&, const Ice::Current&);
};

class BlobjectArrayI : public Ice::BlobjectArray
{
public:

    virtual bool ice_invoke(std::pair<const std::uint8_t*, const std::uint8_t*>, std::vector<std::uint8_t>&,
                            const Ice::Current&);
};

class BlobjectAsyncI : public Ice::BlobjectAsync
{
public:

    virtual void ice_invokeAsync(std::vector<std::uint8_t>,
                                 std::function<void(bool, const std::vector<std::uint8_t>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);
};

class BlobjectArrayAsyncI : public Ice::BlobjectArrayAsync
{
public:

    virtual void ice_invokeAsync(std::pair<const std::uint8_t*, const std::uint8_t*>,
                                 std::function<void(bool, const std::pair<const std::uint8_t*, const std::uint8_t*>&)>,
                                 std::function<void(std::exception_ptr)>,
                                 const Ice::Current&);
};

#endif
