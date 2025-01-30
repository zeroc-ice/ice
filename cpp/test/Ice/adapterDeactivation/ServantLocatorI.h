// Copyright (c) ZeroC, Inc.

#ifndef SERVANT_LOCATOR_I_H
#define SERVANT_LOCATOR_I_H

#include "Ice/Ice.h"

class ServantLocatorI final : public Ice::ServantLocator
{
public:
    ServantLocatorI();
    ~ServantLocatorI() final;

    Ice::ObjectPtr locate(const Ice::Current&, std::shared_ptr<void>&) final;
    void finished(const Ice::Current&, const Ice::ObjectPtr&, const std::shared_ptr<void>&) final;
    void deactivate(std::string_view) final;

public:
    bool _deactivated{false};
    Ice::ObjectPtr _router;
};

#endif
