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

#ifndef FREEZE_INDEX_I_H
#define FREEZE_INDEX_I_H

#include <Ice/Ice.h>
#include <Freeze/Index.h>
#include <Freeze/EvictorI.h>

namespace Freeze
{

class IndexI
{
public:
    
    IndexI(Index&, const std::string&);

    std::vector<Ice::Identity>
    untypedFindFirst(const Freeze::Key&, Ice::Int) const;
    
    std::vector<Ice::Identity>
    untypedFind(const Freeze::Key&) const;
    
    Ice::Int
    untypedCount(const Freeze::Key&) const;
    
    void
    associate(EvictorI* evictor, DbTxn* txn, bool createDb, bool populateIndex);

    int
    secondaryKeyCreate(Db*, const Dbt*, const Dbt*, Dbt*);

    void
    close();
    
   

private:

    Index& _index;
    std::string _name;
    std::auto_ptr<Db> _db;
    EvictorI* _evictor;
};

}
#endif

