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

#ifndef HELLO_ICE
#define HELLO_ICE

#include <Glacier/Session.ice>

class HelloSession implements Glacier::Session
{
    void hello();
};

#endif
