// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SlicedData.h>
#include <Ice/Object.h>

using namespace std;
using namespace Ice;

IceUtil::Shared* Ice::upCast(SliceInfo* p) { return p; }
IceUtil::Shared* Ice::upCast(SlicedData* p) { return p; }

Ice::SlicedData::SlicedData(const SliceInfoSeq& seq) :
    slices(seq)
{
    //
    // Check if any of the preserved slices contain object references.
    //
    _hasObjects = false;
    for(SliceInfoSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        if(!(*p)->objects.empty())
        {
            _hasObjects = true;
            break;
        }
    }
}

void
Ice::SlicedData::clearObjects()
{
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        //
        // Don't just call (*p)->objects.clear(), as releasing references
        // to the objects could have unexpected side effects. We exchange
        // the vector into a temporary and then let the temporary fall out
        // of scope.
        //
        vector<ObjectPtr> tmp;
        tmp.swap((*p)->objects);
    }

    _hasObjects = false;
}

void
Ice::SlicedData::__gcReachable(IceInternal::GCCountMap& m) const
{
    //
    // Iterate over the object references in each preserved slice.
    //
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        for(vector<ObjectPtr>::const_iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            (*q)->__addObject(m);
        }
    }
}

void
Ice::SlicedData::__gcClear()
{
    //
    // Iterate over the object references in each preserved slice.
    //
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        for(vector<ObjectPtr>::iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            if((*q)->__usesGC())
            {
                (*q)->__decRefUnsafe();
                (*q).__clearHandleUnsafe();
            }
        }
        (*p)->objects.clear();
    }

    _hasObjects = false;
}

void
Ice::SlicedData::__addObject(IceInternal::GCCountMap& m)
{
    IceInternal::GCCountMap::iterator pos = m.find(this);
    if(pos == m.end())
    {
        m[this] = 1;
    }
    else
    {
        ++pos->second;
    }
}
