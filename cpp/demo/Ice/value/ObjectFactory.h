// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
