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

#ifndef HELLO_SERVICE_I_H
#define HELLO_SERVICE_I_H

#include <IceBox/IceBox.h>

#ifndef HELLO_API
#   define HELLO_API ICE_DECLSPEC_EXPORT
#endif


class HELLO_API HelloServiceI : public ::IceBox::Service
{
public:

    HelloServiceI();
    virtual ~HelloServiceI();

    virtual void start(const ::std::string&,
		       const ::Ice::CommunicatorPtr&,
		       const ::Ice::StringSeq&);

    virtual void stop();

private:

    ::Ice::ObjectAdapterPtr _adapter;
};

#endif
