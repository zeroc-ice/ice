// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_MAP_H
#define FREEZE_MAP_H

#include <Ice/Ice.h>
#include <iterator>
#include <Freeze/DB.h>
#include <Freeze/Exception.h>
#include <Freeze/Connection.h>

//
// Berkeley DB's DbEnv
//
class DbEnv;

namespace Freeze
{

class IteratorHelper;
class MapHelper;

class MapIndexI;
class MapHelperI;
class IteratorHelperI;
class SharedDb;

class FREEZE_API MapIndexBase : public IceUtil::Shared
{
public:
    
    virtual ~MapIndexBase();

    const std::string& name() const;

    IteratorHelper* untypedFind(const Key&, bool) const;
    int untypedCount(const Key&) const;

    //
    // Implemented by the generated code
    //
    virtual void marshalKey(const Value&, Key&) const = 0;

protected:

    MapIndexBase(const std::string&);

    Ice::CommunicatorPtr _communicator;

private:

    friend class MapHelperI;
    friend class IteratorHelperI;
    friend class SharedDb;

    std::string _name;
    MapIndexI* _impl;
    const MapHelperI* _map;
};

typedef IceUtil::Handle<MapIndexBase> MapIndexBasePtr;


class FREEZE_API MapHelper
{
public:
    
    static MapHelper*
    create(const Freeze::ConnectionPtr& connection, 
	   const std::string& dbName,
	   const std::string& key,
	   const std::string& value,
	   const std::vector<MapIndexBasePtr>&,
	   bool createDb);

    virtual ~MapHelper() = 0;

    virtual IteratorHelper*
    find(const Key&, bool) const = 0;

    virtual void
    put(const Key&, const Value&) = 0;

    virtual size_t
    erase(const Key&) = 0;

    virtual size_t
    count(const Key&) const = 0;
    
    virtual void
    clear() = 0;

    virtual void
    destroy() = 0;

    virtual size_t
    size() const = 0;

    virtual void
    closeAllIterators() = 0;

    virtual const MapIndexBasePtr&
    index(const std::string&) const = 0;
};


class FREEZE_API IteratorHelper
{  
public:

    static IteratorHelper* 
    create(const MapHelper& m, bool readOnly);

    virtual 
    ~IteratorHelper() = 0;

    virtual IteratorHelper*
    clone() const = 0;
    
    virtual const Key* 
    get() const = 0;

    virtual void
    get(const Key*&, const Value*&) const = 0;
    
    virtual  void 
    set(const Value&) = 0;

    virtual void
    erase() = 0;

    virtual bool
    next() const = 0;
}; 


//
// Forward declaration
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class Map;
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class ConstIterator;

//
// This is necessary for MSVC support.
//
struct IteratorBase
{
    typedef std::forward_iterator_tag iterator_category;
};

//
// Database iterator. This implements a forward iterator with the
// restriction that it's only possible to explicitely write back into
// the database.
//
// Two iterators are equal if they use the same database and their
// current records have the same key.
//
// TODO: It's possible to implement bidirectional iterators, if
// necessary.
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class Iterator : public IteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    Iterator(MapHelper& mapHelper, const Ice::CommunicatorPtr& communicator) :
	_helper(IteratorHelper::create(mapHelper, false)),
	_communicator(communicator),
	_refValid(false)
    {
    }

    Iterator(IteratorHelper* helper, const Ice::CommunicatorPtr& communicator) :
	_helper(helper),
	_communicator(communicator),
	_refValid(false)
    {
    }

    Iterator() :
        _refValid(false)
    {
    }

    Iterator(const Iterator& rhs) :
	_communicator(rhs._communicator),
        _refValid(false)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
    }

    Iterator& operator=(const Iterator& rhs)
    {
	if(this != &rhs)
	{
	    if(rhs._helper.get() != 0)
	    {
		_helper.reset(rhs._helper->clone());
	    }
	    else
	    {
		_helper.reset();
	    }
	    _communicator = rhs._communicator;
	    _refValid = false;
	}

	return *this;
    }

    ~Iterator()
    {
    }

    bool operator==(const Iterator& rhs) const
    {
	if(_helper.get() == rhs._helper.get())
	{
	    return true;
	}
	
	if(_helper.get() != 0 && rhs._helper.get() != 0)
	{
	    const Key* lhsKey = _helper->get();
	    const Key* rhsKey = rhs._helper->get();
	    
	    if(lhsKey != 0 && rhsKey != 0)
	    {
		return *lhsKey == *rhsKey;
	    }
	}
	return false;
    }

    bool operator!=(const Iterator& rhs) const
    {
	return !(*this == rhs);
    }

    Iterator& operator++()
    {
	incr();
	return *this;
    }

    Iterator operator++(int)
    {
	Iterator tmp = *this;
	incr();
	return tmp;
    }

    //
    // Note that this doesn't follow the regular iterator mapping:
    //
    // value_type& operator*(), value_type operator*() const
    //
    value_type& operator*() const
    {
        if(!_refValid)
        {
            key_type key;
            mapped_type value;

            getCurrentValue(key, value);

            //
            // !IMPORTANT!
            //
            // This method has to cache the returned value to implement
            // operator->().
            //
            const_cast<key_type&>(_ref.first) = key;
            const_cast<mapped_type&>(_ref.second) = value;
            _refValid = true;
        }

	return _ref;
    }

    value_type* operator->() { return &(operator*()); }

    //
    // This special method allows writing back into the database.
    //
    void set(const mapped_type& value)
    {
	assert(_helper.get());

	Value v;
	ValueCodec::write(value, v, _communicator);
	_helper->set(v);
        _refValid = false;
    }

private:

    void incr()
    {
	assert(_helper.get() != 0);
	if(!_helper->next())
	{
	    //
	    // The iterator has been moved past the end, and is now
	    // invalid.
	    //
	    _helper.reset();
	}
        _refValid = false;
    }

    void getCurrentValue(key_type& key, mapped_type& value) const
    {
	assert(_helper.get() != 0);

	const Key* k = 0;
	const Value* v = 0;
	_helper->get(k, v);
	assert(k != 0);
	assert(v != 0);

	KeyCodec::read(key, *k, _communicator);
	ValueCodec::read(value, *v, _communicator);
    }

    friend class ConstIterator<key_type, mapped_type, KeyCodec, ValueCodec>;
    friend class Map<key_type, mapped_type, KeyCodec, ValueCodec>;

    std::auto_ptr<IteratorHelper> _helper;
    Ice::CommunicatorPtr _communicator;
    //
    // Cached last return value. This is so that operator->() can
    // actually return a pointer. The cached value is reused across
    // multiple calls to operator->() if _refValid is true, which
    // avoids problems in certain situations. For example, if
    // _ref.second is an STL container and you use an STL algorithm
    // such as transform, STLport (debug build) asserts that the
    // addresses of the containers are the same. This would fail if
    // the same value was not returned on subsequent calls to
    // operator->().
    //
    mutable value_type _ref;
    mutable bool _refValid;
};

//
// See Iterator comments for design notes
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class ConstIterator : public IteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    ConstIterator(MapHelper& mapHelper, const Ice::CommunicatorPtr& communicator) :
	_helper(IteratorHelper::create(mapHelper, true)), 
	_communicator(_communicator),
	_refValid(false)
    {
    }

    ConstIterator(IteratorHelper* helper, const Ice::CommunicatorPtr& communicator) :
	_helper(helper),
	_communicator(communicator),
	_refValid(false)
    {
    }

    ConstIterator() :
        _refValid(false)
    {
    }

    ConstIterator(const ConstIterator& rhs) :
	_communicator(rhs._communicator),
        _refValid(false)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
    }

    //
    // A Iterator can be converted to a ConstIterator (but not
    // vice versa) - same for operator=.
    //
    ConstIterator(const Iterator<key_type, mapped_type, KeyCodec, ValueCodec>& rhs) :
        _refValid(false)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
	_communicator = rhs._communicator;
    }

    ConstIterator& operator=(const ConstIterator& rhs)
    {
	if(this != &rhs)
	{
	    if(rhs._helper.get() != 0)
	    {
		_helper.reset(rhs._helper->clone());
	    }
	    else
	    {
		_helper.reset();
	    }
	    _communicator = rhs._communicator;
	    _refValid = false;
	}

	return *this;
    }

    //
    // Create const_iterator from iterator.
    //
    ConstIterator& operator=(const Iterator<key_type, mapped_type, KeyCodec, ValueCodec>& rhs)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
	else
	{
	    _helper.reset();
	}
	_communicator = rhs._communicator;
        _refValid = false;

	return *this;
    }

    ~ConstIterator()
    {
    }

    bool operator==(const ConstIterator& rhs)
    {
	if(_helper.get() == rhs._helper.get())
	{
	    return true;
	}
	
	if(_helper.get() != 0 && rhs._helper.get() != 0)
	{
	    const Key* lhsKey = _helper->get();
	    const Key* rhsKey = rhs._helper->get();
	    
	    if(lhsKey != 0 && rhsKey != 0)
	    {
		return *lhsKey == *rhsKey;
	    }
	}
	return false;
    }

    bool operator!=(const ConstIterator& rhs)
    {
	return !(*this == rhs);
    }

    ConstIterator& operator++()
    {
	incr();
	return *this;
    }

    ConstIterator operator++(int)
    {
	ConstIterator tmp = *this;
	incr();
	return tmp;
    }

    //
    // Note that this doesn't follow the regular iterator mapping:
    //
    // value_type operator*() const
    //
    value_type& operator*() const
    {
        if(!_refValid)
        {
            key_type key;
            mapped_type value;
            
            getCurrentValue(key, value);

            //
            // !IMPORTANT!
            //
            // This method has to cache the returned value to implement
            // operator->().
            //
            const_cast<key_type&>(_ref.first) = key;
            const_cast<mapped_type&>(_ref.second) = value;
            _refValid = true;
        }

	return _ref;
    }

    pointer operator->() const { return &(operator*()); }

private:

    void incr()
    {
	assert(_helper.get() != 0);
	if(!_helper->next())
	{
	    //
	    // The iterator has been moved past the end, and is now
	    // invalid.
	    //
	    _helper.reset();
	}
        _refValid = false;
    }

    void getCurrentValue(key_type& key, mapped_type& value) const
    {
	assert(_helper.get() != 0);

	const Key* k = 0;
	const Value* v = 0;
	_helper->get(k, v);
	assert(k != 0);
	assert(v != 0);

	KeyCodec::read(key, *k, _communicator);
	ValueCodec::read(value, *v, _communicator);
    }

    friend class Map<key_type, mapped_type, KeyCodec, ValueCodec>;

    std::auto_ptr<IteratorHelper> _helper;
    Ice::CommunicatorPtr _communicator;

    //
    // Cached last return value. This is so that operator->() can
    // actually return a pointer. The cached value is reused across
    // multiple calls to operator->() if _refValid is true, which
    // avoids problems in certain situations. For example, if
    // _ref.second is an STL container and you use an STL algorithm
    // such as transform, STLport (debug build) asserts that the
    // addresses of the containers are the same. This would fail
    // if the same value was not returned on subsequent calls
    // to operator->().
    //
    mutable value_type _ref;
    mutable bool _refValid;
};

//
// This is an STL container that matches the requirements of a
// Associated Container - with the restriction that operator[] isn't
// implemented. It also supports the same interface as a Hashed
// Associative Container (with the above restrictions), except the
// hasher & key_equal methods.
//
// TODO: If necessary it would be possible to implement reverse and
// bidirectional iterators.
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class Map
{
public:

    typedef std::pair<const key_type, const mapped_type> value_type;

    //
    // These are not supported:
    //
    // hasher, key_equal, key_compare, value_compare
    //

    typedef Iterator<key_type, mapped_type, KeyCodec, ValueCodec > iterator;
    typedef ConstIterator<key_type, mapped_type, KeyCodec, ValueCodec > const_iterator;

    //
    // No definition for reference, const_reference, pointer or
    // const_pointer.
    //

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    //
    // Allocators are not supported.
    //
    // allocator_type
    //

    //
    // Constructors
    //
    Map(const Freeze::ConnectionPtr& connection, 
	const std::string& dbName, 
	bool createDb = true) :
	_communicator(connection->getCommunicator())
    {
	std::vector<MapIndexBasePtr> indices;
	_helper.reset(MapHelper::create(connection, dbName, 
					KeyCodec::typeId(), ValueCodec::typeId(), 
					indices, createDb));
    }

    template <class _InputIterator>
    Map(const Freeze::ConnectionPtr& connection, 
	const std::string& dbName, 
	bool createDb,
	_InputIterator first, _InputIterator last) :
	_communicator(connection->getCommunicator())
    {
	std::vector<MapIndexBasePtr> indices;
	_helper.reset(MapHelper::create(connection, dbName, 
					KeyCodec::typeId(), ValueCodec::typeId(),
					indices, createDb));
	while(first != last)
	{
	    put(*first);
	    ++first;
	}
    }

    ~Map()
    {
    }

    //
    // Neither of these operations are supported.
    //
    // key_compare key_comp() const, value_compare value_comp() const,
    // hasher hash_funct() const, key_equal key_eq() const
    //

    bool operator==(const Map& rhs) const
    {
	//
	// This does a memberwise equality for the entire contents of
	// the database. While slow this is always correct. Database
	// equality is not necessarily correct in the context of a
	// transaction.
	//
	if(count() != rhs.count())
	{
	    return false;
	}

	for(const_iterator p = rhs.begin() ; p != rhs.end() ; ++p)
	{
	    const_iterator q = rhs.find(p->first);
	    if(q == rhs.end())
	    {
		return false;
	    }
	    if(p->second != q->second)
	    {
		return false;
	    }
	}
	return true;
    }

    bool operator!=(const Map& rhs) const
    {
	return !(*this == rhs);
    }
    
    void swap(Map& rhs)
    {
	MapHelper* tmp = _helper.release();
	_helper.reset(rhs._helper.release());
	rhs._helper.reset(tmp);
	
	Ice::CommunicatorPtr tmpCom = _communicator;
	_communicator = rhs._communicator;
	rhs._communicator = tmpCom;
    }

    iterator begin()
    {
	try
	{
	    return iterator(IteratorHelper::create(*_helper.get(), false), _communicator);
	}
	catch(const NotFoundException&)
	{
	    return iterator();
	}
    }
    const_iterator begin() const
    {
	try
	{
	    return const_iterator(IteratorHelper::create(*_helper.get(), true), _communicator);
	}
	catch(const NotFoundException&)
	{
	    return const_iterator();
	}
    }

    iterator end()
    {
	return iterator();
    }

    const_iterator end() const
    {
	return const_iterator();
    }

    bool empty() const
    {
	return size() == 0;
    }

    size_type size() const
    {
	return _helper->size();
    }

    size_type max_size() const
    {
	return 0xffffffff; // TODO: is this the max?
    }

    //
    // This method isn't implemented.
    //
    // mapped_type& operator[](const key_type& key)
    //

    //
    // This method isn't in the STLport library - but it's referenced
    // in "STL Tutorial and Refrence Guide, Second Edition". It's not
    // currently implemented.
    //
    // const mapped_type& operator[](const key_type& key) const;
    //

    //
    // No allocators.
    //
    //allocator_type get_allocator() const;
    //
 
    iterator insert(iterator /*position*/, const value_type& key)
    {
	//
	// position is ignored.
	//
	Key k;
	KeyCodec::write(key.first, k, _communicator);
	
	iterator r = iterator(_helper->find(k, false), _communicator);

	if(r == end())
	{
	    Value v;
	    ValueCodec::write(key.second, v, _communicator);
	    
	    _helper->put(k, v);
	    r = iterator(_helper->find(k, false), _communicator);
	}

	return r;
    }

    std::pair<iterator, bool> insert(const value_type& key)
    {
	Key k;
	KeyCodec::write(key.first, k, _communicator);

	iterator r = iterator(_helper->find(k, false), _communicator);
	bool inserted = false;

	if(r == end())
	{
	    Value v;
	    ValueCodec::write(key.second, v, _communicator);
	    
	    _helper->put(k, v);
	    inserted = true;
	    r = iterator(_helper->find(k, false), _communicator);
	}

	return std::pair<iterator, bool>(r, inserted);
    }

    template <typename InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
	while(first != last)
	{
	    insert(*first);
	    ++first;
	}
    }

    void put(const value_type& key)
    {
	//
	// insert or replace
	//
	Key k;
	Value v;
	KeyCodec::write(key.first, k, _communicator);
	ValueCodec::write(key.second, v, _communicator);

	_helper->put(k, v);
    }

    template <typename InputIterator>
    void put(InputIterator first, InputIterator last)
    {
	while(first != last)
	{
	    put(*first);
	    ++first;
	}
    }

    void erase(iterator position)
    {
	assert(position._helper.get() != 0);
	position._helper->erase();
    }

    size_type erase(const key_type& key)
    {
	Key k;
	KeyCodec::write(key, k, _communicator);

	return _helper->erase(k);
    }

    void erase(iterator first, iterator last)
    {
	while(first != last)
	{
	    first._helper->erase();
	    ++first;
	}
    }

    void clear()
    {
	_helper->clear();
    }

    
    //
    // destroy is not in STL
    //
    void destroy()
    {
	_helper->destroy();
    }
    
    iterator find(const key_type& key)
    {
	Key k;
	KeyCodec::write(key, k, _communicator);

	return iterator(_helper->find(k, false), _communicator);
    }

    const_iterator find(const key_type& key) const
    {
	Key k;
	KeyCodec::write(key, k, _communicator);

	return const_iterator(_helper->find(k, true), _communicator);
    }

    size_type count(const key_type& key) const
    {
	Key k;
	KeyCodec::write(key, k, _communicator);
	
	return _helper->count(k);

    }

    std::pair<iterator, iterator> equal_range(const key_type& key)
    {
	iterator p = find(key);
	return std::pair<iterator,iterator>(p,p);
    }

    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
	const_iterator p = find(key);
	return std::pair<const_iterator,const_iterator>(p,p);
    }

    const Ice::CommunicatorPtr&
    communicator() const
    {
	return _communicator();
    }


protected:

    Map(const Ice::CommunicatorPtr& communicator) :
	_communicator(communicator)
    {
    }

    std::auto_ptr<MapHelper> _helper;
    const Ice::CommunicatorPtr _communicator;
};


}

//
// This is for MSVC.
//
# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
namespace std
{

// TODO: update.
template <class key_type, class mapped_type, class KeyCodec, class ValueCodec>
inline pair<const key_type, const mapped_type>*
value_type(const Freeze::Iterator<key_type, mapped_type, KeyCodec, ValueCodec>&)
{
    return (pair<const key_type, const mapped_type>*)0;
}

template <class key_type, class mapped_type, class KeyCodec, class ValueCodec>
inline pair<const key_type, const mapped_type>*
value_type(const Freeze::ConstIterator<key_type, mapped_type, KeyCodec, ValueCodec>&)
{
    return (pair<const key_type, const mapped_type>*)0;
}

inline forward_iterator_tag iterator_category(const Freeze::IteratorBase&)
{
    return forward_iterator_tag();
}

inline ptrdiff_t* distance_type(const Freeze::IteratorBase&) { return (ptrdiff_t*) 0; }

}

#endif

#endif
