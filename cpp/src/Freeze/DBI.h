// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_DB_FACTORY_I_H
#define FREEZE_DB_FACTORY_I_H

#include <Freeze/Initialize.h>
#include <Freeze/DB.h>
#include <db.h>

namespace Freeze
{

class DBFactoryI : public DBFactory, public JTCMutex
{
public:
    
    virtual DBPtr createDB();
    virtual void destroy();

private:

    DBFactoryI(const ::Ice::CommunicatorPtr&, const ::Ice::PropertiesPtr&);
    virtual ~DBFactoryI();

    friend FREEZE_API DBFactoryPtr initialize(const ::Ice::CommunicatorPtr&);
    friend FREEZE_API DBFactoryPtr initializeWithProperties(const ::Ice::CommunicatorPtr&,
							    const ::Ice::PropertiesPtr&);

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::PropertiesPtr _properties;
    bool _destroy;
    DB_ENV* _dbenv;
};

}

#endif
