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

#include "stdafx.h"
#include <HelloI.h>

using namespace std;

HelloI::HelloI(const LogIPtr& log, CDialog* dialog) :
    _log(log), _dialog(dialog)
{
}

void
HelloI::sayHello(const Ice::Current&) const
{
    _log->message("Hello World!");
}

void
HelloI::shutdown(const Ice::Current&)
{
    if(_dialog)
    {
        _log->message("Shutting down...");
        _dialog->EndDialog(0);
        _dialog = 0;
    }
}
