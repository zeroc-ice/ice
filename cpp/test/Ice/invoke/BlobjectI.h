//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Object.h>

class BlobjectI final : public Ice::Blobject
{
public:

    bool ice_invoke(std::vector<std::uint8_t>, std::vector<std::uint8_t>&, const Ice::Current&) final;
};

class BlobjectArrayI final : public Ice::BlobjectArray
{
public:

    bool ice_invoke(
        std::pair<const std::uint8_t*, const std::uint8_t*>,
        std::vector<std::uint8_t>&,
        const Ice::Current&) final;
};

class BlobjectAsyncI final : public Ice::BlobjectAsync
{
public:

    void ice_invokeAsync(
        std::vector<std::uint8_t>,
        std::function<void(bool, const std::vector<std::uint8_t>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;
};

class BlobjectArrayAsyncI final : public Ice::BlobjectArrayAsync
{
public:

    void ice_invokeAsync(
        std::pair<const std::uint8_t*, const std::uint8_t*>,
        std::function<void(bool, const std::pair<const std::uint8_t*, const std::uint8_t*>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;
};

#endif
