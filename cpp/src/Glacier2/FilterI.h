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

#include <Glacier2/Session.h>

#include <Ice/Identity.h>
#include <string>
#include <vector>
#include <list>

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

    FilterT(const std::vector<T>&, const std::vector<T>&, const bool);

    //
    // Slice to C++ mapping.
    //
    virtual void addAccept(const std::vector<T>&, const Ice::Current&);
    virtual void removeAccept(const std::vector<T>&, const Ice::Current&);

    virtual void setAccept(const std::vector<T>&, const Ice::Current&);
    virtual std::vector<T> getAccept(const Ice::Current&) const;

    virtual void addReject(const std::vector<T>&, const Ice::Current&);
    virtual void removeReject(const std::vector<T>&, const Ice::Current&);
    
    virtual void setReject(const std::vector<T>&, const Ice::Current&);
    virtual std::vector<T> getReject(const Ice::Current&) const;

    virtual bool getAcceptOverride(const Ice::Current&) const;
    virtual void setAcceptOverride(bool value, const Ice::Current&);

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
    
    std::vector<T> _accept;
    std::vector<T> _reject;
    bool _acceptOverride;

    void 
    addImpl(std::vector<T>& dest, const std::vector<T>& additions)
    {
	//
	// Sort the filter elements first, erasing duplicates. Then we can
	// simply use the STL merge algorithm to add to our list of filters.
	//
	std::vector<T> newItems(additions);
	sort(newItems.begin(), newItems.end());
	newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());

	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	std::vector<T> merged(dest.size() + newItems.size());
	merge(newItems.begin(), newItems.end(), dest.begin(), dest.end(), merged.begin());
	merged.erase(unique(merged.begin(), merged.end()), merged.end());
	swap(dest, merged);
    }

    void 
    removeImpl(std::vector<T>& dest, const std::vector<T>& deletions)
    {
	//
	// Our removal algorithm depends on the filter elements to be
	// removed to be sorted in the same order as our current elements.
	//
	std::vector<T> toRemove(dest);
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
	iterator mark = dest.begin();
	std::list<iterator> deleteList;

	while(r != toRemove.end())
	{
	    iterator i = mark;
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

	for(literator i = deleteList.begin(); i != deleteList.end(); ++i)
	{
	    dest.erase(*i);
	}
    }
};

template<class T, class P>
FilterT<T, P>::FilterT(const std::vector<T>& accept, const std::vector<T>& reject, const bool acceptOverride):
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
FilterT<T, P>::addAccept(const std::vector<T>& additions, const Ice::Current&)
{
    addImpl(_accept, additions);
}

template<class T, class P> void
FilterT<T, P>::removeAccept(const std::vector<T>& deletions, const Ice::Current&)
{
    removeImpl(_accept, deletions);
}

template<class T, class P> void
FilterT<T, P>::setAccept(const std::vector<T>& filterElements, const Ice::Current&)
{
    std::vector<T> newItems(filterElements);
    sort(newItems.begin(), newItems.end());
    newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    swap(newItems, _accept);
}

template<class T, class P> std::vector<T>
FilterT<T, P>::getAccept(const Ice::Current&) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    return _accept;
}

template<class T, class P> void
FilterT<T, P>::addReject(const std::vector<T>& additions, const Ice::Current&)
{
    addImpl(_reject, additions);
}

template<class T, class P> void
FilterT<T, P>::removeReject(const std::vector<T>& deletions, const Ice::Current&)
{
    removeImpl(_reject, deletions);
}

template<class T, class P> void
FilterT<T, P>::setReject(const std::vector<T>& filterElements, const Ice::Current&)
{
    std::vector<T> newItems(filterElements);
    sort(newItems.begin(), newItems.end());
    newItems.erase(unique(newItems.begin(), newItems.end()), newItems.end());
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    swap(newItems, _reject);
}

template<class T, class P> std::vector<T>
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

typedef FilterT<Ice::Identity, Glacier2::IdentityFilter> IdentityFilterI;
typedef IceUtil::Handle< FilterT<Ice::Identity, Glacier2::IdentityFilter> > IdentityFilterIPtr;

typedef FilterT<std::string, Glacier2::StringFilter> StringFilterI;
typedef IceUtil::Handle< FilterT<std::string, Glacier2::StringFilter> > StringFilterIPtr;


};

#endif
