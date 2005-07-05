// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
HelloI::sayHello(const IceE::Current&) const
{
    _log->message("Hello World!");
}

void
HelloI::shutdown(const IceE::Current&)
{
    if(_dialog)
    {
        _log->message("Shutting down...");
        _dialog->EndDialog(0);
        _dialog = 0;
    }
}
