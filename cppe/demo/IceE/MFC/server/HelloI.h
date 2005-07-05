// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>
#include <LogI.h>

class CDialog;

class HelloI : public Demo::Hello
{
public:

    HelloI(const LogIPtr&, CDialog*);

    virtual void sayHello(const IceE::Current&) const;
    virtual void shutdown(const IceE::Current&);

private:

    LogIPtr _log;
    CDialog* _dialog;
};

#endif
