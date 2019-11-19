//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SERVER_BLOBJECT_H
#define SERVER_BLOBJECT_H

#include <Glacier2/Blobject.h>

namespace Glacier2
{

class ServerBlobject final : public Glacier2::Blobject
{
public:

    ServerBlobject(std::shared_ptr<Instance>, std::shared_ptr<Ice::Connection>);
    ~ServerBlobject() override;

    void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*> inEncaps,
                         std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                         std::function<void(std::exception_ptr)> error,
                         const Ice::Current& current) override;
};

}

#endif
