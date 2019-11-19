//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
#ifndef FILTER_I_H
#define FILTER_I_H

#include <Glacier2/Session.h>

#include <Ice/Identity.h>

#include <list>
#include <mutex>
#include <string>
#include <vector>

namespace Glacier2
{

template <typename T, class P>
class FilterT : public P
{
public:

    FilterT(const std::vector<T>&);

    //
    // Slice to C++ mapping.
    //
    virtual void add(std::vector<T>, const Ice::Current&) override;
    virtual void remove(std::vector<T>, const Ice::Current&) override;
    virtual std::vector<T> get(const Ice::Current&) override;

    //
    // Internal functions.
    //
    bool
    match(const T& candidate) const
    {
        std::lock_guard<std::mutex> lg(_mutex);
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
        std::lock_guard<std::mutex> lg(_mutex);
        return _items.size() == 0;
    }

private:

    std::vector<T> _items;

    mutable std::mutex _mutex;
};

template<class T, class P>
FilterT<T, P>::FilterT(const std::vector<T>& accept):
    _items(accept)
{
    sort(_items.begin(), _items.end());
    _items.erase(unique(_items.begin(), _items.end()), _items.end());
}

template<class T, class P> void
FilterT<T, P>::add(std::vector<T> additions, const Ice::Current&)
{
    //
    // Sort the filter elements first, erasing duplicates. Then we can
    // simply use the STL merge algorithm to add to our list of filters.
    //
    std::vector<T> newItems(additions);
    sort(newItems.begin(), newItems.end());
    newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());

    std::lock_guard<std::mutex> lg(_mutex);
    std::vector<T> merged(_items.size() + newItems.size());
    merge(newItems.begin(), newItems.end(), _items.begin(), _items.end(), merged.begin());
    merged.erase(unique(merged.begin(), merged.end()), merged.end());
    swap(_items, merged);
}

template<class T, class P> void
FilterT<T, P>::remove(std::vector<T> deletions, const Ice::Current&)
{
    //
    // Our removal algorithm depends on the filter elements to be
    // removed to be sorted in the same order as our current elements.
    //
    std::vector<T> toRemove(deletions);
    sort(toRemove.begin(), toRemove.end());
    toRemove.erase(unique(toRemove.begin(), toRemove.end()), toRemove.end());

    std::lock_guard<std::mutex> lg(_mutex);

    //
    // Our vectors are both sorted, so if we keep track of our first
    // match between the current set and the set of items to be removed,
    // we do not need to traverse the whole of the current set each
    // time. We also use a list of deletions instead of erasing things
    // itemwise.
    //

    typename std::vector<T>::const_iterator r = toRemove.begin();
    typename std::vector<T>::iterator mark = _items.begin();
    std::list<typename std::vector<T>::iterator> deleteList;

    while(r != toRemove.end())
    {
        typename std::vector<T>::iterator i = mark;
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

    for(const auto& item : deleteList)
    {
        _items.erase(item);
    }
}

template<class T, class P> std::vector<T>
FilterT<T, P>::get(const Ice::Current&)
{
    std::lock_guard<std::mutex> lg(_mutex);
    return _items;
}

using IdentitySetI = FilterT<Ice::Identity, Glacier2::IdentitySet>;
using StringSetI = FilterT<std::string, Glacier2::StringSet>;

};

#endif
