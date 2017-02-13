// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPHP_COMMUNICATOR_H
#define ICEPHP_COMMUNICATOR_H

#include <Config.h>
#include <Ice/CommunicatorF.h>

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

bool communicatorInit(TSRMLS_D);
bool communicatorShutdown(TSRMLS_D);
bool communicatorRequestInit(TSRMLS_D);
bool communicatorRequestShutdown(TSRMLS_D);

//
// Class entry.
//
extern zend_class_entry* communicatorClassEntry;

//
// The CommunicatorInfo class represents a communicator that is in use by a PHP request.
//
class CommunicatorInfo : public IceUtil::Shared
{
public:

    virtual void getZval(zval* TSRMLS_DC) = 0;
    virtual void addRef(TSRMLS_D) = 0;
    virtual void decRef(TSRMLS_D) = 0;

    virtual Ice::CommunicatorPtr getCommunicator() const = 0;
};
typedef IceUtil::Handle<CommunicatorInfo> CommunicatorInfoPtr;

} // End of namespace IcePHP

#endif
