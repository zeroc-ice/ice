// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef HELLO_SERVICE_I_H
#define HELLO_SERVICE_I_H

#include <IceBox/IceBox.h>

class HelloServiceI : public ::IceBox::Service
{
public:

    HelloServiceI();
    virtual ~HelloServiceI();

    virtual void init(const ::std::string&,
                      const ::Ice::CommunicatorPtr&,
                      const ::Ice::PropertiesPtr&,
                      const ::Ice::StringSeq&);

    virtual void start();

    virtual void stop();

private:

    ::Ice::ObjectAdapterPtr _adapter;
};

#endif
