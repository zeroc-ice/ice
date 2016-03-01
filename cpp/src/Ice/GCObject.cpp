// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/GCObject.h>

#include <set>
#include <stack>

using namespace std;
using namespace IceUtil;
using namespace IceInternal;

namespace 
{

typedef ::map<GCObject*, int> GCCountMap;
Mutex* gcMutex = 0;

class Init
{
public:

    Init()
    {
        gcMutex = new Mutex();
    }

    ~Init()
    {
        delete gcMutex;
        gcMutex = 0;
    }
};

Init init;
    
class ClearMembers : public GCVisitor
{
public:
    
    virtual bool visit(GCObject*);
};
ClearMembers clearMembers;

class DecreaseRefCounts : public GCVisitor
{
public:
    
    DecreaseRefCounts(GCCountMap&);

    virtual bool visit(GCObject*);

private:

    GCCountMap& _counts;
};

class RestoreRefCountsIfReachable : public GCVisitor
{
public:
        
    RestoreRefCountsIfReachable(GCCountMap&);

    virtual bool visit(GCObject*);

private:

    GCCountMap& _counts;
    bool _reachable;
};

class MarkCollectable : public GCVisitor
{
public:

    MarkCollectable();

    virtual bool visit(GCObject*);

    void visitNeighbor(GCObject*);

private:

    int _counter;
    map<GCObject*, int> _numbers;
    stack<GCObject*> _p;
    stack<GCObject*> _s;

    class VisitNeighbors : public IceInternal::GCVisitor
    {
    public:

        void setVisitor(MarkCollectable*);
        virtual bool visit(GCObject*);

    private:

        MarkCollectable* _visitor;
    };
    VisitNeighbors _neighborsVisitor;
};

class ClearCollectable : public GCVisitor
{
public:
    
    virtual bool visit(GCObject*);
};

}

bool
ClearMembers::visit(GCObject* obj)
{
    return true;
}

DecreaseRefCounts::DecreaseRefCounts(GCCountMap& counts) : _counts(counts)
{
}

bool 
DecreaseRefCounts::visit(GCObject* obj)
{
    //
    // Visit the object only once when the object is inserted for
    // the first time in the counts map. After, we just decrement
    // its reference count. Decrementing the reference counts of
    // reachable objects will indicate when a cycle is
    // collectable. Collectable objects are those with a reference
    // count of zero and for which there's no "reachable" parent
    // object (objects with a reference count > 0).
    //
    GCCountMap::iterator p = _counts.find(obj);
    if(p == _counts.end())
    {
        _counts.insert(make_pair(obj, obj->__getRefUnsafe() - 1));
        if(obj->__hasFlag(GCObject::Collectable))
        {
            obj->__gcVisitMembers(*this);
        }
    }
    else
    {
        --p->second;
    }
    return false;
}

RestoreRefCountsIfReachable::RestoreRefCountsIfReachable(GCCountMap& counts) : _counts(counts), _reachable(false)
{
}

bool 
RestoreRefCountsIfReachable::visit(GCObject* obj)
{
    GCCountMap::iterator p = _counts.find(obj);
    if(p == _counts.end())
    {
        //
        // If the object has been removed from the counts map,
        // it's reachable.
        //
        return false;
    } 
    else if(_reachable)
    {
        //
        // If parent object is reachable, this object is also
        // reachable. Remove it from the counts map and also make
        // reachable children.
        //
        _counts.erase(p);
        obj->__gcVisitMembers(*this);
    }
    else if(p->second == 0)
    {
        //
        // If the object is collectable, set its count to -1 to
        // indicate that it was already visited prevent it from
        // being visited again.
        //
        p->second = -1;
        obj->__gcVisitMembers(*this);
    }
    else if(p->second > 0)
    {
        //
        // Object isn't collectable, remove it from the counts map
        // and visit its sub-graph to remove children wobjects from
        // the counts map since they are also reachable.
        //
        _counts.erase(p); 
            
        _reachable = true;
        obj->__gcVisitMembers(*this);
        _reachable = false;
    }
    return false;
}

MarkCollectable::MarkCollectable() : _counter(0)
{
    _neighborsVisitor.setVisitor(this);
}

bool 
MarkCollectable::visit(GCObject* obj)
{
    //
    // Set the collectable flag on the object graph. While setting the
    // flag, we also check if the object graph has cycles and mark all the
    // objects which are part of a cycle with the CycleMember flag.
    //
    // We use the path-based strong component algorithm to detect the
    // strong components of the graph.
    //

    if(obj->__hasFlag(GCObject::Collectable))
    {
        return false;
    }
    obj->__setFlag(GCObject::Collectable);
        
    _numbers[obj] = ++_counter;
    _p.push(obj);
    _s.push(obj);

    obj->__gcVisitMembers(_neighborsVisitor);

    if(_p.top() == obj)
    {
        GCObject* o;
        do
        {
            o = _s.top();
            _s.pop();
            o->__setFlag(GCObject::CycleMember);
        }
        while(o != obj);
        _p.pop();
    }
    return false;
}

void
MarkCollectable::visitNeighbor(GCObject* obj)
{
    map<GCObject*, int>::const_iterator p = _numbers.find(obj);
    if(p == _numbers.end())
    {
        visit(obj);
    }
    else if(!obj->__hasFlag(GCObject::CycleMember))
    {
        while(_numbers[_p.top()] > p->second)
        {
            _p.pop();
        }
    }
}

void
MarkCollectable::VisitNeighbors::setVisitor(MarkCollectable* visitor)
{
    _visitor = visitor;
}

bool 
MarkCollectable::VisitNeighbors::visit(GCObject* obj)
{
    _visitor->visitNeighbor(obj);
    return false;
}

bool 
ClearCollectable::visit(GCObject* obj)
{
    //
    // Clear the collectable flag on the object graph.
    //
    if(obj->__hasFlag(GCObject::Collectable))
    {
        obj->__clearFlag(GCObject::Collectable | GCObject::CycleMember);
        obj->__gcVisitMembers(*this);
    }
    return false;
}

//
// Flags constant used for collection of graphs.
//
const unsigned char GCObject::Collectable = 2;
const unsigned char GCObject::CycleMember = 4;
const unsigned char GCObject::Visiting = 8;

//
// GCObject
//
void
IceInternal::GCObject::__incRef()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(gcMutex);
    ++_ref;
}

void
IceInternal::GCObject::__decRef()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(gcMutex);
    bool doDelete = false;
    assert(_ref > 0);

    //
    // Try to collect the object each time its reference count is
    // decremented and only if it's part of a cycle.
    //
    if(_ref > 1 && __hasFlag(CycleMember) && collect(lock))
    {
        return;
    }

    if(--_ref == 0)
    {
        doDelete = !__hasFlag(NoDelete);
        __setFlag(NoDelete);
    }

    lock.release();
    if(doDelete)
    {
        delete this;
    }
}

int
IceInternal::GCObject::__getRef() const
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(gcMutex);
    return _ref;
}

void
IceInternal::GCObject::__setNoDelete(bool b)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(gcMutex);
    IceUtil::Shared::__setNoDelete(b);
}

bool
GCObject::__gcVisit(GCVisitor& v)
{
    return v.visit(this);
}

void
GCObject::ice_collectable(bool enable)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(gcMutex);
    if(enable)
    {
        ClearCollectable().visit(this);
        MarkCollectable().visit(this);
    }
    else
    {
        ClearCollectable().visit(this);
    }
}

bool
GCObject::collect(IceUtilInternal::MutexPtrLock<IceUtil::Mutex>& lock)
{
    GCCountMap counts;

    //
    // Go through the object graph and decrease reference counts for
    // all the objects from the graph. Cycles which can be collected
    // should lead to objects with a zero reference count.
    //
    DecreaseRefCounts(counts).visit(this);
    assert(counts.find(this) != counts.end());
    if(counts[this] > 0)
    {
        return false; // Object is still reachable, we're done.
    }

    //
    // Go the graph again and check for objects which are still
    // reachable. If there are any, we restore the reference counts
    // for the sub-graph of the reachable object and we remove the
    // reachable objects from the counts map. At the end, if the
    // counts map is empty, it indicates that this object graph isn't
    // collectable yet.
    //
    RestoreRefCountsIfReachable(counts).visit(this);
    if(counts.empty())
    {
        return false;
    }

    assert(counts.find(this) != counts.end()); // This object must be collectable.

    //
    // At this point, we can release the lock. The objects from counts
    // are un-reachable from the user code and clearing the members of
    // the collectable objects requires acquiring the mutex to
    // decrement their reference counts.
    //
    lock.release();

    //
    // Break all the cyclic reference counts of objects which are
    // remaining in the counts map by clearing members.
    //
    // We first go through the list to mark all the objects as
    // non-deletable and we also disable collection for all those
    // objects since we already know they are collectable.
    //
    // After clearing members, we delete all the collectable
    // objects. We can't just delete the objects since those objects
    // likely point to each other.
    //
    for(GCCountMap::const_iterator p = counts.begin(); p != counts.end(); ++p)
    {
        p->first->__setFlag(NoDelete);
        p->first->__clearFlag(CycleMember); // Disable cycle collection.
    }
    for(GCCountMap::const_iterator p = counts.begin(); p != counts.end(); ++p)
    {
        p->first->__gcVisitMembers(clearMembers);
    }
    for(GCCountMap::const_iterator p = counts.begin(); p != counts.end(); ++p)
    {
        delete p->first;
    }
    return true;
}
