// Copyright (c) ZeroC, Inc.

#ifndef SERVANT_LOCATOR_I_H
#define SERVANT_LOCATOR_I_H

#include "Ice/Ice.h"

namespace Test
{
    class ServantLocatorI : public Ice::ServantLocator
    {
    public:
        ServantLocatorI(std::string);
        ~ServantLocatorI() override;
        Ice::ObjectPtr locate(const Ice::Current&, std::shared_ptr<void>&) override;
        void finished(const Ice::Current&, const Ice::ObjectPtr&, const std::shared_ptr<void>&) override;
        void deactivate(std::string_view) override;

    protected:
        virtual Ice::ObjectPtr newServantAndCookie(std::shared_ptr<void>&) const = 0;
        virtual void checkCookie(const std::shared_ptr<void>&) const = 0;
        virtual void throwTestIntfUserException() const = 0;

    private:
        void exception(const Ice::Current&);

        const std::string _category;
        bool _deactivated{false};
        std::int32_t _requestId{-1};
    };

};

#endif
