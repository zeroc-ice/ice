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
#include <Freeze/Freeze.h>
#include <vector>

namespace Freeze
{

class ObjectStore;

class EvictorIteratorI : public EvictorIterator
{
public:

    EvictorIteratorI(ObjectStore*, Ice::Int);

    virtual bool hasNext();
    virtual Ice::Identity next();

private:

    std::vector<Ice::Identity>::const_iterator
    nextBatch();

    ObjectStore* _store;
    size_t _batchSize;
    std::vector<Ice::Identity>::const_iterator _batchIterator;

    Key _key;
    std::vector<Ice::Identity> _batch;
    bool _more;
    bool _initialized;
};

}

#endif
