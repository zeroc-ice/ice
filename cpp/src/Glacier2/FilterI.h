// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
#ifndef FILTER_I_H
#define FILTER_I_H

#include <Ice/Identity.h>
#include <Glacier2/Session.h>

namespace Glacier2
{

template <class T, class P>
class FilterT : public P, public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    FilterT(const T&, const T&, const bool);

    //
    // Slice to C++ mapping.
    //
    virtual void addAccept(const T&, const Ice::Current&);
    virtual void removeAccept(const T&, const Ice::Current&);

    virtual void setAccept(const T&, const Ice::Current&);
    virtual T getAccept(const Ice::Current&) const;

    virtual void addReject(const T&, const Ice::Current&);
    virtual void removeReject(const T&, const Ice::Current&);
    
    virtual void setReject(const T&, const Ice::Current&);
    virtual T getReject(const Ice::Current&) const;

    virtual bool getAcceptOverride(const Ice::Current&) const;
    virtual void setAcceptOverride(bool value, const Ice::Current&);

    //
    // Internal functions.
    //
    bool
    match(const T::value_type& candidate) const
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	//
	// Empty vectors mean no filtering, so all matches will succeed.
	//
	if(_accept.size() == 0 && _reject.size() == 0)
	{
	    return true;
	}

	bool result;
	
	if(_reject.size() == 0)
	{
	    //
	    // No reject filters, treat mode as default reject.
	    //
	    result = binary_search(_accept.begin(), _accept.end(), candidate);
	}
	else if(_accept.size() == 0)
	{
	    //
	    // No accept filters, treat mode as default accept.
	    //
	    result = !binary_search(_reject.begin(), _reject.end(), candidate);
	}
	else
	{
	    //
	    // We have both accept and reject filters. We need to consider
	    // the _acceptOverride flag.
	    //
	    if(_acceptOverride)
	    {
		result = !binary_search(_reject.begin(), _reject.end(), candidate);
		if(!result)
		{
		    result = binary_search(_accept.begin(), _accept.end(), candidate);
		}
	    }
	    else
	    {
		result = binary_search(_accept.begin(), _accept.end(), candidate);
		if(result)
		{
		    result = !binary_search(_reject.begin(), _reject.end(), candidate);
		}
		
	    }
	}
	return result;
    }
	
private:
    
    T _accept;
    T _reject;
    bool _acceptOverride;

    void addImpl(T&, const T&);
    void removeImpl(T&, const T&);
};

template<class T, class P>
FilterT<T, P>::FilterT(const T& accept, const T& reject, const bool acceptOverride):
    _accept(accept),
    _reject(reject),
    _acceptOverride(acceptOverride)
{
    sort(_accept.begin(), _accept.end());
    _accept.erase(unique(_accept.begin(), _accept.end()), _accept.end());
    sort(_reject.begin(), _reject.end());
    _reject.erase(unique(_reject.begin(), _reject.end()), _reject.end());
}

template<class T, class P> void
FilterT<T, P>::addAccept(const T& additions, const Ice::Current&)
{
    addImpl(_accept, additions);
}

template<class T, class P> void
FilterT<T, P>::removeAccept(const T& deletions, const Ice::Current&)
{
    removeImpl(_accept, deletions);
}

template<class T, class P> void
FilterT<T, P>::setAccept(const T& filterElements, const Ice::Current&)
{
    T newItems(filterElements);
    sort(newItems.begin(), newItems.end());
    newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    swap(newItems, _accept);
}

template<class T, class P> T
FilterT<T, P>::getAccept(const Ice::Current&) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    return _accept;
}

template<class T, class P> void
FilterT<T, P>::addReject(const T& additions, const Ice::Current&)
{
    addImpl(_reject, additions);
}

template<class T, class P> void
FilterT<T, P>::removeReject(const T& deletions, const Ice::Current&)
{
    removeImpl(_reject, deletions);
}

template<class T, class P> void
FilterT<T, P>::setReject(const T& filterElements, const Ice::Current&)
{
    T newItems(filterElements);
    sort(newItems.begin(), newItems.end());
    newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    swap(newItems, _reject);
}

template<class T, class P> T
FilterT<T, P>::getReject(const Ice::Current&) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    return _reject;
}

template<class T, class P> bool
FilterT<T, P>::getAcceptOverride(const Ice::Current&) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    return _acceptOverride;
}

template<class T, class P> void
FilterT<T, P>::setAcceptOverride(bool value, const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    _acceptOverride = value;
}

template<class T, class P> void
FilterT<T, P>::addImpl(T& dest, const T& additions)
{
    //
    // Sort the filter elements first, erasing duplicates. Then we can
    // simply use the STL merge algorithm to add to our list of filters.
    //
    T newItems(additions);
    sort(newItems.begin(), newItems.end());
    newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    T merged(dest.size() + newItems.size());
    merge(newItems.begin(), newItems.end(), dest.begin(), dest.end(), merged.begin());
    merged.erase(unique(merged.begin(), merged.end()), merged.end());
    swap(dest, merged);
}

template<class T, class P> void
FilterT<T, P>::removeImpl(T& dest, const T& deletions)
{
    //
    // Our removal algorithm depends on the filter elements to be
    // removed to be sorted in the same order as our current elements.
    //
    T toRemove(dest);
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
    T::const_iterator r = toRemove.begin();
    T::iterator mark = dest.begin();
    list<T::iterator> deleteList;
    
    while(r != toRemove.end())
    {
	T::iterator i = mark;
	while(i != dest.end() && r != toRemove.end())
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

    for(list<T::iterator>::const_iterator i = deleteList.begin(); i != deleteList.end(); ++i)
    {
	dest.erase(*i);
    }
}

typedef FilterT<Ice::IdentitySeq, Glacier2::IdFilter> IdentityFilterI;
typedef IceUtil::Handle< FilterT<Ice::IdentitySeq, Glacier2::IdFilter> > IdentityFilterIPtr;

typedef FilterT<std::vector<std::string>, Glacier2::StringFilter> StringFilterI;
typedef IceUtil::Handle< FilterT<std::vector<std::string>, Glacier2::StringFilter> > StringFilterIPtr;

};

#endif
