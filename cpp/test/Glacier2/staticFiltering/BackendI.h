// Copyright (c) ZeroC, Inc.

#ifndef BACKEND_I_H
#define BACKEND_I_H

#include "Backend.h"

class BackendI final : public Test::Backend
{
public:
    void shutdown(const Ice::Current&) override;
};

#endif
