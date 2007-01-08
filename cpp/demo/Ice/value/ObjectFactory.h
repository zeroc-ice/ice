// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SERVANT_FACTORY_H
#define SERVANT_FACTORY_H

class ObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();
};

#endif
