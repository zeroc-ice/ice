// Copyright (c) ZeroC, Inc.

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include "Ice/Object.h"

class BlobjectI final : public Ice::Blobject
{
public:
    bool ice_invoke(std::vector<std::byte>, std::vector<std::byte>&, const Ice::Current&) final;
};

class BlobjectArrayI final : public Ice::BlobjectArray
{
public:
    bool ice_invoke(std::pair<const std::byte*, const std::byte*>, std::vector<std::byte>&, const Ice::Current&) final;
};

class BlobjectAsyncI final : public Ice::BlobjectAsync
{
public:
    void ice_invokeAsync(
        std::vector<std::byte>,
        std::function<void(bool, const std::vector<std::byte>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;
};

class BlobjectArrayAsyncI final : public Ice::BlobjectArrayAsync
{
public:
    void ice_invokeAsync(
        std::pair<const std::byte*, const std::byte*>,
        std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;
};

#endif
