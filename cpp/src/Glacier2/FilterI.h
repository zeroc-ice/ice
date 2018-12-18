// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
#ifndef FILTER_I_H
#define FILTER_I_H

#include <Glacier2/Session.h>

#include <Ice/Identity.h>
#include <string>
#include <vector>
#include <list>

#ifdef _MSC_VER
#   pragma warning(disable:4505) // unreferenced local function has been removed
#endif

namespace Glacier2
{

template <typename T, class P>
class FilterT : public P, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    //
    // These typedefs are a compiler workaround for GCC and iterators
    // depending on nested templates.
    //
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::list<iterator>::iterator literator;

    FilterT(const std::vector<T>&);

    //
    // Slice to C++ mapping.
    //
    virtual void add(const std::vector<T>&, const Ice::Current&);
    virtual void remove(const std::vector<T>&, const Ice::Current&);
    virtual std::vector<T> get(const Ice::Current&);

    //
    // Internal functions.
    //
    bool
    match(const T& candidate) const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
        //
        // Empty vectors mean no filtering, so all matches will succeed.
        //
        if(_items.size() == 0)
        {
            return true;
        }

        return binary_search(_items.begin(), _items.end(), candidate);
    }

    bool
    empty() const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
        return _items.size() == 0;
    }

private:

    std::vector<T> _items;
};

template<class T, class P>
FilterT<T, P>::FilterT(const std::vector<T>& accept):
    _items(accept)
{
    sort(_items.begin(), _items.end());
    _items.erase(unique(_items.begin(), _items.end()), _items.end());
}

template<class T, class P> void
FilterT<T, P>::add(const std::vector<T>& additions, const Ice::Current&)
{
    //
    // Sort the filter elements first, erasing duplicates. Then we can
    // simply use the STL merge algorithm to add to our list of filters.
    //
    std::vector<T> newItems(additions);
    sort(newItems.begin(), newItems.end());
    newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    std::vector<T> merged(_items.size() + newItems.size());
    merge(newItems.begin(), newItems.end(), _items.begin(), _items.end(), merged.begin());
    merged.erase(unique(merged.begin(), merged.end()), merged.end());
    swap(_items, merged);
}

template<class T, class P> void
FilterT<T, P>::remove(const std::vector<T>& deletions, const Ice::Current&)
{
    //
    // Our removal algorithm depends on the filter elements to be
    // removed to be sorted in the same order as our current elements.
    //
    std::vector<T> toRemove(deletions);
    sort(toRemove.begin(), toRemove.end());
    toRemove.erase(unique(toRemove.begin(), toRemove.end()), toRemove.end());

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    //
    // Our vectors are both sorted, so if we keep track of our first
    // match between the current set and the set of items to be removed,
    // we do not need to traverse the whole of the current set each
    // time. We also use a list of deletions instead of erasing things
    // itemwise.
    //

    //
    // The presence of the 'typename' is a GCC specific workaround. The
    // iterator types apparently resolve to a 'void' in GCC type
    // causing compiler errors.
    //
    const_iterator r = toRemove.begin();
    iterator mark = _items.begin();
    std::list<iterator> deleteList;

    while(r != toRemove.end())
    {
        iterator i = mark;
        while(i != _items.end() && r != toRemove.end())
        {
            if(*r == *i)
            {
                //
                // We want this list to be in LIFO order because we are
                // going to erase things from the tail forward.
                //
                deleteList.push_front(i);
                ++i;
                ++r;
                mark = i;
            }
            else
            {
                ++i;
            }
        }

        if(r == toRemove.end())
        {
            break;
        }
        ++r;
    }

    for(literator i = deleteList.begin(); i != deleteList.end(); ++i)
    {
        _items.erase(*i);
    }
}

template<class T, class P> std::vector<T>
FilterT<T, P>::get(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    return _items;
}

typedef FilterT<Ice::Identity, Glacier2::IdentitySet> IdentitySetI;
typedef IceUtil::Handle< FilterT<Ice::Identity, Glacier2::IdentitySet> > IdentitySetIPtr;

typedef FilterT<std::string, Glacier2::StringSet> StringSetI;
typedef IceUtil::Handle< FilterT<std::string, Glacier2::StringSet> > StringSetIPtr;

};

#endif
