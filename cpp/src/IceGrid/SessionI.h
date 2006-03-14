// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_SESSIONI_H
#define ICEGRID_SESSIONI_H

#include <Glacier2/Session.h>

namespace IceGrid
{

class SessionI : virtual public Glacier2::Session
{
public:

    virtual IceUtil::Time timestamp() const = 0;
    

};
typedef IceUtil::Handle<SessionI> SessionIPtr;

}

#endif
