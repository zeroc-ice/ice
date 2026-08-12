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
    // Called when the PHP module is loaded and unloaded, i.e. once per process. communicatorShutdown destroys the
    // communicators that outlived the requests that created them.
    bool communicatorInit(void);
    bool communicatorShutdown(void);

    // Called at the start and end of each PHP request. communicatorRequestShutdown releases the request's hold on
    // the communicators it used; a registered communicator survives until it expires or the module shuts down.
    bool communicatorRequestInit(void);
    bool communicatorRequestShutdown(void);

    // Class entry.
    extern zend_class_entry* communicatorClassEntry;

    // The CommunicatorInfo class represents a communicator that is in use by a PHP request.
    class CommunicatorInfo
    {
    public:
        virtual void getZval(zval*) = 0;

        virtual Ice::CommunicatorPtr getCommunicator() const = 0;
        virtual Ice::SliceLoaderPtr getSliceLoader() const = 0;
    };
    using CommunicatorInfoPtr = std::shared_ptr<CommunicatorInfo>;

} // End of namespace IcePHP

#endif
