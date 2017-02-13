// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_EVICTOR_ITERATOR_I_H
#define FREEZE_EVICTOR_ITERATOR_I_H

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <vector>

namespace Freeze
{

class TransactionI;
typedef IceUtil::Handle<TransactionI> TransactionIPtr;


class ObjectStoreBase;

class EvictorIteratorI : public EvictorIterator
{
public:

    EvictorIteratorI(ObjectStoreBase*, const TransactionIPtr&, Ice::Int);

    virtual bool hasNext();
    virtual Ice::Identity next();

private:

    std::vector<Ice::Identity>::const_iterator
    nextBatch();

    ObjectStoreBase* _store;
    size_t _batchSize;
    std::vector<Ice::Identity>::const_iterator _batchIterator;

    Key _key;
    std::vector<Ice::Identity> _batch;
    bool _more;
    bool _initialized;
    TransactionIPtr _tx;
};

}

#endif
