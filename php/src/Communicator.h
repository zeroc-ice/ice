// Copyright (c) ZeroC, Inc.

#ifndef ICEPHP_COMMUNICATOR_H
#define ICEPHP_COMMUNICATOR_H

#include "Config.h"
#include "Ice/CommunicatorF.h"

//
// Global functions.
//
extern "C"
{
    ZEND_FUNCTION(Ice_initialize);
    ZEND_FUNCTION(Ice_register);
    ZEND_FUNCTION(Ice_unregister);
    ZEND_FUNCTION(Ice_find);
    ZEND_FUNCTION(Ice_getProperties);
    ZEND_FUNCTION(Ice_identityToString);
    ZEND_FUNCTION(Ice_stringToIdentity);
}

namespace IcePHP
{
    bool communicatorInit(void);
    bool communicatorShutdown(void);
    bool communicatorRequestInit(void);
    bool communicatorRequestShutdown(void);

    // Class entry.
    extern zend_class_entry* communicatorClassEntry;

    // The CommunicatorInfo class represents a communicator that is in use by a PHP request.
    class CommunicatorInfo
    {
    public:
        virtual void getZval(zval*) = 0;
        virtual void addRef(void) = 0;
        virtual void decRef(void) = 0;

        virtual Ice::CommunicatorPtr getCommunicator() const = 0;
    };
    using CommunicatorInfoPtr = std::shared_ptr<CommunicatorInfo>;

} // End of namespace IcePHP

#endif
