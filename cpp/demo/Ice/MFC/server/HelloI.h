// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>
#include <LogI.h>

class CDialog;

class HelloI : public Hello
{
public:

    HelloI(const LogIPtr&, CDialog*);

    virtual void sayHello(const Ice::Current&) const;
    virtual void shutdown(const Ice::Current&);

private:

    LogIPtr _log;
    CDialog* _dialog;
};

#endif
