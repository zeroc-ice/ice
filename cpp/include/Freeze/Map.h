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

#ifndef FREEZE_MAP_H
#define FREEZE_MAP_H

#include <Ice/Ice.h>
#include <iterator>
#include <Freeze/DB.h>
#include <Freeze/DBException.h>

//
// Berkeley DB's DbEnv
//
class DbEnv;

namespace Freeze
{

class DBIteratorHelper;

class FREEZE_API DBMapHelper
{
public:
    
    static DBMapHelper*
    create(const Ice::CommunicatorPtr& communicator, 
	   const std::string& envName, 
	   const std::string& dbName, 
	   bool createDb);

    static DBMapHelper*
    create(const Ice::CommunicatorPtr& communicator, 
	   DbEnv& dbEnv, 
	   const std::string& dbName, 
	   bool createDb);

    virtual ~DBMapHelper() = 0;

    virtual DBIteratorHelper*
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

    const Ice::CommunicatorPtr&
    getCommunicator() const
    {
	return _communicator;
    }

protected:

    DBMapHelper(const Ice::CommunicatorPtr&);

    Ice::CommunicatorPtr _communicator;
};


class FREEZE_API DBIteratorHelper
{  
public:

    static DBIteratorHelper* 
    create(const DBMapHelper& m, bool readOnly);

    virtual 
    ~DBIteratorHelper() = 0;

    virtual DBIteratorHelper*
    clone() const = 0;
    
    virtual void
    get(const Key*&, const Value*&) const = 0;
    
    virtual  void 
    set(const Value&) = 0;

    virtual void
    erase() = 0;

    virtual bool
    next() const = 0;

    virtual bool
    equals(const DBIteratorHelper&) const = 0;
    
    virtual const Ice::CommunicatorPtr&
    getCommunicator() const = 0;
}; 



//
// Forward declaration
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class DBMap;
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class ConstDBIterator;

//
// This is necessary for MSVC support.
//
struct DBIteratorBase
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
class DBIterator : public DBIteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    DBIterator(DBMapHelper& mapHelper) :
	_helper(DBIteratorHelper::create(mapHelper, false)), 
	_refValid(false)
    {
    }

    DBIterator(DBIteratorHelper* helper) :
	_helper(helper), 
	_refValid(false)
    {
    }

    DBIterator() :
        _refValid(false)
    {
    }

    DBIterator(const DBIterator& rhs) :
        _refValid(false)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
    }

    DBIterator& operator=(const DBIterator& rhs)
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
	    
	    _refValid = false;
	}

	return *this;
    }

    ~DBIterator()
    {
    }

    bool operator==(const DBIterator& rhs) const
    {
	if(_helper.get() != 0 && rhs._helper.get() != 0)
	{
	    return _helper->equals(*rhs._helper.get());
	}
	else
	{
	    return _helper.get() == rhs._helper.get();
	}
    }

    bool operator!=(const DBIterator& rhs) const
    {
	return !(*this == rhs);
    }

    DBIterator& operator++()
    {
	incr();
	return *this;
    }

    DBIterator operator++(int)
    {
	DBIterator tmp = *this;
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
	ValueCodec::write(value, v, _helper->getCommunicator());
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

	const Ice::CommunicatorPtr& communicator = _helper->getCommunicator();
	KeyCodec::read(key, *k, communicator);
	ValueCodec::read(value, *v, communicator);
    }

    friend class ConstDBIterator<key_type, mapped_type, KeyCodec, ValueCodec>;
    friend class DBMap<key_type, mapped_type, KeyCodec, ValueCodec>;

    std::auto_ptr<DBIteratorHelper> _helper;

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
// See DBIterator comments for design notes
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class ConstDBIterator : public DBIteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    ConstDBIterator(DBMapHelper& mapHelper) :
	_helper(DBIteratorHelper::create(mapHelper, true)), 
	_refValid(false)
    {
    }

    ConstDBIterator(DBIteratorHelper* helper) :
	_helper(helper), 
	_refValid(false)
    {
    }

    ConstDBIterator() :
        _refValid(false)
    {
    }

    ConstDBIterator(const ConstDBIterator& rhs) :
        _refValid(false)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
    }

    //
    // A DBIterator can be converted to a ConstDBIterator (but not
    // vice versa) - same for operator=.
    //
    ConstDBIterator(const DBIterator<key_type, mapped_type, KeyCodec, ValueCodec>& rhs) :
        _refValid(false)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
    }

    ConstDBIterator& operator=(const ConstDBIterator& rhs)
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
	    
	    _refValid = false;
	}

	return *this;
    }

    //
    // Create const_iterator from iterator.
    //
    ConstDBIterator& operator=(const DBIterator<key_type, mapped_type, KeyCodec, ValueCodec>& rhs)
    {
	if(rhs._helper.get() != 0)
	{
	    _helper.reset(rhs._helper->clone());
	}
	else
	{
	    _helper.reset();
	}

        _refValid = false;

	return *this;
    }

    ~ConstDBIterator()
    {
    }

    bool operator==(const ConstDBIterator& rhs)
    {
	if(_helper.get() != 0 && rhs._helper.get() != 0)
	{
	    return _helper->equals(*rhs._helper);
	}
	else
	{
	    return _helper.get() == rhs._helper.get();
	}
    }

    bool operator!=(const ConstDBIterator& rhs)
    {
	return !(*this == rhs);
    }

    ConstDBIterator& operator++()
    {
	incr();
	return *this;
    }

    ConstDBIterator operator++(int)
    {
	ConstDBIterator tmp = *this;
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

	const Ice::CommunicatorPtr& communicator = _helper->getCommunicator();
	KeyCodec::read(key, *k, communicator);
	ValueCodec::read(value, *v, communicator);
    }

    friend class DBMap<key_type, mapped_type, KeyCodec, ValueCodec>;

    std::auto_ptr<DBIteratorHelper> _helper;

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
class DBMap
{
public:

    typedef std::pair<const key_type, const mapped_type> value_type;

    //
    // These are not supported:
    //
    // hasher, key_equal, key_compare, value_compare
    //

    typedef DBIterator<key_type, mapped_type, KeyCodec, ValueCodec > iterator;
    typedef ConstDBIterator<key_type, mapped_type, KeyCodec, ValueCodec > const_iterator;

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
    DBMap(const Ice::CommunicatorPtr& communicator, 
	  const std::string& envName, 
	  const std::string& dbName, 
	  bool createDb = true) :
	_helper(DBMapHelper::create(communicator, envName, dbName, createDb))
    {
    }

    DBMap(const Ice::CommunicatorPtr& communicator, 
	  DbEnv& dbEnv, 
	  const std::string& dbName, 
	  bool createDb = true) :
	_helper(DBMapHelper::create(communicator, dbEnv, dbName, createDb))
    {
    }

    template <class _InputDBIterator>
    DBMap(const Ice::CommunicatorPtr& communicator, 
	  const std::string& envName, 
	  const std::string& dbName, 
	  bool createDb,
	  _InputDBIterator first, _InputDBIterator last) :
	_helper(new DBMapHelper(communicator, envName, dbName, createDb))
    {
	while(first != last)
	{
	    put(*first);
	    ++first;
	}
    }

    template <class _InputDBIterator>
    DBMap(const Ice::CommunicatorPtr& communicator, 
	  DbEnv& dbEnv, 
	  const std::string& dbName, 
	  bool createDb,
	  _InputDBIterator first, _InputDBIterator last) :
	_helper(new DBMapHelper(communicator, dbEnv, dbName, createDb))
    {
	while(first != last)
	{
	    put(*first);
	    ++first;
	}
    }

    ~DBMap()
    {
    }

    //
    // Neither of these operations are supported.
    //
    // key_compare key_comp() const, value_compare value_comp() const,
    // hasher hash_funct() const, key_equal key_eq() const
    //

    bool operator==(const DBMap& rhs) const
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

    bool operator!=(const DBMap& rhs) const
    {
	return !(*this == rhs);
    }
    
    void swap(DBMap& rhs)
    {
	DBMapHelper* tmp = _helper.release();
	_helper.reset(rhs._helper.release());
	rhs._helper.reset(tmp);
    }

    iterator begin()
    {
	try
	{
	    return iterator(DBIteratorHelper::create(*_helper.get(), false));
	}
	catch(const DBNotFoundException&)
	{
	    return iterator();
	}
    }
    const_iterator begin() const
    {
	try
	{
	    return const_iterator(DBIteratorHelper::create(*_helper.get(), true));
	}
	catch(const DBNotFoundException&)
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
	const Ice::CommunicatorPtr& communicator = _helper->getCommunicator();

	Key k;
	KeyCodec::write(key.first, k, communicator);
	
	iterator r = iterator(_helper->find(k, false));

	if(r == end())
	{
	    Value v;
	    ValueCodec::write(key.second, v, communicator);
	    
	    _helper->put(k, v);
	    r = iterator(_helper->find(k, false));
	}

	return r;
    }

    std::pair<iterator, bool> insert(const value_type& key)
    {
	const Ice::CommunicatorPtr& communicator = _helper->getCommunicator();

	Key k;
	KeyCodec::write(key.first, k, communicator);

	iterator r = iterator(_helper->find(k, false));
	bool inserted = false;

	if(r == end())
	{
	    Value v;
	    ValueCodec::write(key.second, v, communicator);
	    
	    _helper->put(k, v);
	    inserted = true;
	    r = iterator(_helper->find(k, false));
	}

	return std::pair<iterator, bool>(r, inserted);
    }

    template <typename InputDBIterator>
    void insert(InputDBIterator first, InputDBIterator last)
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
	const Ice::CommunicatorPtr& communicator = _helper->getCommunicator();

	Key k;
	Value v;
	KeyCodec::write(key.first, k, communicator);
	ValueCodec::write(key.second, v, communicator);

	_helper->put(k, v);
    }

    template <typename InputDBIterator>
    void put(InputDBIterator first, InputDBIterator last)
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
	KeyCodec::write(key, k, _helper->getCommunicator());

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
	KeyCodec::write(key, k, _helper->getCommunicator());

	return iterator(_helper->find(k, false));
    }

    const_iterator find(const key_type& key) const
    {
	Key k;
	KeyCodec::write(key, k, _helper->getCommunicator());

	return const_iterator(_helper->find(k, true));
    }

    size_type count(const key_type& key) const
    {
	Key k;
	KeyCodec::write(key, k, _helper->getCommunicator());
	
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

private:

    std::auto_ptr<DBMapHelper> _helper;
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
value_type(const Freeze::DBIterator<key_type, mapped_type, KeyCodec, ValueCodec>&)
{
    return (pair<const key_type, const mapped_type>*)0;
}

template <class key_type, class mapped_type, class KeyCodec, class ValueCodec>
inline pair<const key_type, const mapped_type>*
value_type(const Freeze::ConstDBIterator<key_type, mapped_type, KeyCodec, ValueCodec>&)
{
    return (pair<const key_type, const mapped_type>*)0;
}

inline forward_iterator_tag iterator_category(const Freeze::DBIteratorBase&)
{
    return forward_iterator_tag();
}

inline ptrdiff_t* distance_type(const Freeze::DBIteratorBase&) { return (ptrdiff_t*) 0; }

}

#endif

#endif
