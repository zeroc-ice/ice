// **********************************************************************
//
// Copyright (c) 2001
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

#ifndef SERVANT_FACTORY_H
#define SERVANT_FACTORY_H

class ObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();
};

#endif
