// Copyright (c) ZeroC, Inc.

#ifndef SERVER_BLOBJECT_H
#define SERVER_BLOBJECT_H

#include "Blobject.h"

namespace Glacier2
{
    class ServerBlobject final : public Glacier2::Blobject
    {
    public:
        ServerBlobject(std::shared_ptr<Instance>, Ice::ConnectionPtr);

        void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*> inEncaps,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)> response,
            std::function<void(std::exception_ptr)> error,
            const Ice::Current& current) final;
    };
}

#endif
