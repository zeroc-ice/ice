// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FACTORY_H
#define FACTORY_H

class Factory : public Ice::ValueFactory
{
public:

    virtual Ice::ObjectPtr create(const std::string&);
};

#endif
