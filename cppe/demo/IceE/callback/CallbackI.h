// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_I_H
#define CALLBACK_I_H

#include <Callback.h>

class CallbackI : public Demo::Callback
{
public:

    virtual void initiateCallback(const Demo::CallbackReceiverPrx&, const IceE::Current&);
    virtual void shutdown(const IceE::Current&);
};

#endif
