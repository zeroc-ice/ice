//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GC_OBJECT_H
#define ICE_GC_OBJECT_H

#ifndef ICE_CPP11_MAPPING

#include <Ice/Object.h>

#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Mutex.h>

namespace IceInternal
{

class GCObject;

class GCVisitor
{
public:

    virtual ~GCVisitor()
    {
    }

    virtual bool visit(GCObject*) = 0;
};

class ICE_API GCObject : public virtual Ice::Object
{
public:

    //
    // Flags constant used for collection of graphs
    //
    static const unsigned char Collectable;
    static const unsigned char CycleMember;
    static const unsigned char Visiting;

    //
    // Override IceUtil::Shared methods
    //
    virtual void __incRef();
    virtual void __decRef();
    virtual int __getRef() const;
    virtual void __setNoDelete(bool);

    //
    // Override Object methods
    //
    virtual bool _iceGcVisit(GCVisitor&);
    virtual void ice_collectable(bool);

    //
    // This method is implemented by Slice classes to visit class
    // members.
    //
    virtual void _iceGcVisitMembers(IceInternal::GCVisitor&) = 0;

    int _iceGetRefUnsafe()
    {
        return _ref;
    }

private:

    bool collect(IceUtilInternal::MutexPtrLock<IceUtil::Mutex>&);
};

}

#endif

#endif
