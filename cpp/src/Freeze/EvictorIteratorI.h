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

#ifndef FREEZE_EVICTOR_ITERATOR_I_H
#define FREEZE_EVICTOR_ITERATOR_I_H

#include <Ice/Ice.h>
#include <Freeze/Evictor.h>
#include <Freeze/DB.h>
#include <vector>

namespace Freeze
{

class EvictorI;

class EvictorIteratorI : public EvictorIterator
{
public:

    EvictorIteratorI(EvictorI&, Ice::Int, bool);

    virtual bool hasNext();
    virtual Ice::Identity next();

private:

    std::vector<Ice::Identity>::const_iterator
    nextBatch();

    EvictorI& _evictor;
    size_t _batchSize;
    bool _loadServants;
    std::vector<Ice::Identity>::const_iterator _batchIterator;

    Key _key;
    Value _value;
    std::vector<Ice::Identity> _batch;
    bool _more;
};

}

#endif
