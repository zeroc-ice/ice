// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
