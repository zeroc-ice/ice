// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_MAP_H
#define FREEZE_MAP_H

#include <Freeze/DB.h>
#include <Ice/Ice.h>

#include <iterator>

namespace Freeze
{

//
// This wrapper class holds a key/value pair and the associated
// database. Upon assignment it writes the new value into the backing
// database.
//
template <typename key_type, typename value_type, typename KeyCodec, typename ValueCodec>
class DBWrapper
{
public:

    DBWrapper()
    {
    }

    DBWrapper(const DBPtr& db, const key_type& key, const value_type& value) :
	_db(db), _key(key), _value(value)
    {
    }

    operator value_type() const
    {
	return _value;
    }

    //
    // For MSVC (EvictorI.cpp). TODO: Re-examine.
    //
    value_type get_value() const
    {
	return _value;
    }

    bool operator!=(const value_type& value) const
    {
	return _value != value;
    }

    bool operator==(const value_type& value) const
    {
	return _value == value;
    }

    DBWrapper& operator=(const value_type& value)
    {
	Freeze::Key k;
	Freeze::Value v;
	
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());
	k = KeyCodec::write(_key, instance);
	v = ValueCodec::write(value, instance);
	
	_db->put(k, v);

	_value = value;

	return *this;
    }

private:

    DBPtr _db;
    key_type _key;
    value_type _value;
};

//
// Forward declaration
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class DBMap;
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class ConstDBIterator;

//
// It's necessary to have DBPair so that automatic conversion to
// std::pair<key_type, mapped_type> from std::pair<key_type,
// DBWrapper<...> > can work correctly (and global operator== &
// operator!=).
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
struct DBPair
{
    typedef key_type first_type;
    typedef  DBWrapper<key_type, mapped_type, KeyCodec, ValueCodec> second_type;
    
    first_type first;
    second_type second;
    
    DBPair() :
	first(first_type()), second(second_type())
    {
    }
    
    DBPair(const first_type& f, const second_type& s) :
	first(f), second(s)
    {
    }
    
    ~DBPair()
    {
    }

    operator std::pair<key_type, mapped_type>()
    {
	return std::pair<key_type, mapped_type>(first, second);
    }
};

//
// Global operator== & operator!= for both DBPair & std::pair.
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
inline bool operator==(const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p1,
		       const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p2)
{ 
    return p1.first == p2.first && p1.second == p2.second; 
}

template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
inline bool operator==(const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p1,
		       const std::pair<key_type, mapped_type>& p2)
{ 
    return p1.first == p2.first && p1.second == p2.second; 
}

template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
inline bool operator==(const std::pair<key_type, mapped_type>& p1,
		       const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p2)
{ 
    return p1.first == p2.first && p1.second == p2.second; 
}

template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
inline bool operator!=(const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p1,
		       const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p2)
{ 
    return p1.first != p2.first || p1.second != p2.second; 
}

template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
inline bool operator!=(const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p1,
		       const std::pair<key_type, mapped_type>& p2)
{ 
    return p1.first != p2.first || p1.second != p2.second; 
}

template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
inline bool operator!=(const std::pair<key_type, mapped_type>& p1,
		       const DBPair<key_type, mapped_type, KeyCodec, ValueCodec>& p2)
{ 
    return p1.first != p2.first || p1.second != p2.second; 
}

//
// This is necessary for MSVC support.
//
struct DBIteratorBase
{
    typedef std::forward_iterator_tag iterator_category;
};

//
// Database iterator. This implements a forward iterator.
//
// Equality and inequality are based on whether the iterator is
// "valid". An valid iterator contains a valid database and cursor
// pointer, otherwise the iterator is invalid.
//
// TODO: It's possible to implement bidirectional iterators, if
// necessary.
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class DBIterator : public DBIteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    //
    // NOTE:
    //
    // Normally this would be const key_type, mapped_type. However,
    // since there the returned data is always data that is in the
    // backing database then it's not necessary - in fact, it screws
    // up the operator*() and operator-> methods.
    //
    typedef std::pair<key_type, mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    //
    // This is a special value-type that allows write-back to the
    // database. It's necessary to use DBPair so that automatic
    // conversion to std::pair<key_type,mapped_type> can work correct
    // (and global operator== & !=).
    //
    typedef DBPair<key_type, mapped_type, KeyCodec, ValueCodec> reference_value_type;

    DBIterator(const DBPtr& db, const DBCursorPtr& cursor)
	: _db(db), _cursor(cursor)
    {
    }

    DBIterator()
    {
    }

    DBIterator(const DBIterator& rhs)
    {
	if (rhs._cursor)
	{
	    _cursor = rhs._cursor->clone();
	}

	_db = rhs._db;
    }

    DBIterator& operator=(const DBIterator& rhs)
    {
	if (_cursor)
	{
	    _cursor->close();
	}

	if (rhs._cursor)
	{
	    _cursor = rhs._cursor->clone();
	}

	_db = rhs._db;

	return *this;
    }

    ~DBIterator()
    {
	if (_cursor)
	{
	    _cursor->close();
	}
    }

    bool operator==(const DBIterator& rhs)
    {
	if (!_db && !rhs._db)
	{
	    return true;
	}
	return false;
    }

    bool operator!=(const DBIterator& rhs)
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
	tmp.incr();
	return tmp;
    }

    //
    // Note that this doesn't follow the regular iterator mapping:
    //
    // value_type& operator*(), value_type operator*() const
    //
    reference_value_type& operator*() const
    {
	key_type key;
	mapped_type value;

	getCurrentValue(key, value);
	_ref = reference_value_type(key, DBWrapper<key_type, mapped_type, KeyCodec, ValueCodec> (_db, key, value));
	return _ref;
    }

    //
    // Special version that allows writing back to the database.
    //
    reference_value_type* operator->() { return &(operator*()); }

private:

    void incr()
    {
	assert(_cursor && _db);
	if (!_cursor->next())
	{
	    //
	    // The iterator has been moved past the end, and is now
	    // invalid.
	    //
	    _cursor->close();
	    _cursor = 0;
	    _db = 0;
	}
    }

    void getCurrentValue(key_type& key, mapped_type& value) const
    {
	Freeze::Key k;
	Freeze::Value v;
	
	_cursor->curr(k, v);

	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());
	KeyCodec::read(key, k, instance);
	ValueCodec::read(value, v, instance);
    }

    friend class ConstDBIterator<key_type, mapped_type, KeyCodec, ValueCodec>;
    friend class DBMap<key_type, mapped_type, KeyCodec, ValueCodec>;

    DBPtr _db;
    DBCursorPtr _cursor;

    //
    // Cached last return value. This is so that operator->() can
    // actually return a pointer.
    //
    mutable reference_value_type _ref;
};

//
// See DBIterator comments for design notes
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class ConstDBIterator : public DBIteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    //
    // NOTE:
    //
    // Normally this would be const key_type, mapped_type. However,
    // since there the returned data is always data that is in the
    // backing database then it's not necessary - in fact, it screws
    // up the operator*() and operator-> methods.
    //
    typedef std::pair<key_type, mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    ConstDBIterator(const DBPtr& db, const DBCursorPtr& cursor)
	: _db(db), _cursor(cursor)
    {
    }
    ConstDBIterator() { }

    ConstDBIterator(const ConstDBIterator& rhs)
    {
	if (rhs._cursor)
	{
	    _cursor = rhs._cursor->clone();
	}

	_db = rhs._db;
    }

    //
    // A DBIterator can be converted to a ConstDBIterator (but not
    // vice versa) - same for operator=.
    //
    ConstDBIterator(const DBIterator<key_type, mapped_type, KeyCodec, ValueCodec>& rhs)
    {
	if (rhs._cursor)
	{
	    _cursor = rhs._cursor->clone();
	}

	_db = rhs._db;
    }

    ConstDBIterator& operator=(const ConstDBIterator& rhs)
    {
	if (_cursor)
	{
	    _cursor->close();
	}

	if (rhs._cursor)
	{
	    _cursor = rhs._cursor->clone();
	}

	_db = rhs._db;

	return *this;
    }

    //
    // Create const_iterator from iterator.
    //
    ConstDBIterator& operator=(const DBIterator<key_type, mapped_type, KeyCodec, ValueCodec>& rhs)
    {
	if (_cursor)
	{
	    _cursor->close();
	}

	if (rhs._cursor)
	{
	    _cursor = rhs._cursor->clone();
	}

	_db = rhs._db;

	return *this;
    }

    ~ConstDBIterator()
    {
	if (_cursor)
	{
	    _cursor->close();
	}
    }

    bool operator==(const ConstDBIterator& rhs)
    {
	if (!_db && !rhs._db)
	{
	    return true;
	}
	return false;
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
	tmp.incr();
	return tmp;
    }

    value_type& operator*() const
    {
	key_type key;
	mapped_type value;

	getCurrentValue(key, value);
	_ref = value_type(key, value);
	return _ref;
    }

    pointer operator->() const { return &(operator*()); }

private:

    void incr()
    {
	assert(_cursor);
	if (!_cursor->next())
	{
	    //
	    // The iterator has been moved past the end, and is now
	    // invalid.
	    //
	    _cursor->close();
	    _cursor = 0;
	    _db = 0;
	}
    }

    void getCurrentValue(key_type& key, mapped_type& value) const
    {
	Freeze::Key k;
	Freeze::Value v;
	
	_cursor->curr(k, v);

	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());
	KeyCodec::read(key, k, instance);
	ValueCodec::read(value, v, instance);
    }

    friend class DBMap<key_type, mapped_type, KeyCodec, ValueCodec>;

    DBPtr _db;
    DBCursorPtr _cursor;

    //
    // Cached last return value. This is so that operator->() can
    // actually return a pointer.
    //
    mutable value_type _ref;
};

//
// This is an STL container that matches the requirements of a
// Associated Container. It also supports the same interface as a
// Hashed Associative Container, except the hasher & key_equal
// methods.
//
// TODO: If necessary it would be possible to implement reverse and
// bidirectional iterators.
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec>
class DBMap
{
public:

    //
    // NOTE:
    //
    // Normally this would be const key_type, mapped_type. However,
    // since there the returned data is always data that is in the
    // backing database then it's not necessary - in fact, it screws
    // up the operator*() and operator-> methods.
    //
    typedef std::pair<key_type, mapped_type> value_type;

    //
    // These are not supported:
    //
    // hasher, key_equal, key_compare, value_compare
    //

    typedef DBIterator<key_type, mapped_type, KeyCodec, ValueCodec > iterator;
    typedef ConstDBIterator<key_type, mapped_type, KeyCodec, ValueCodec > const_iterator;

    typedef std::pair<const key_type&, mapped_type>& reference;
    typedef const std::pair<const key_type&, mapped_type>& const_reference;

    typedef std::pair<const key_type&, mapped_type>* pointer;
    typedef const std::pair<const key_type&, mapped_type>* const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    //
    // Special type similar to DBIterator::value_type_reference
    //
    typedef DBWrapper<key_type, mapped_type, KeyCodec, ValueCodec> mapped_type_reference;

    //
    // Allocators are not supported.
    //
    // allocator_type

    //
    // Constructors
    //
    DBMap(const DBPtr& db) :
	_db(db)
    {
    }

#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    DBMap(const DBPtr& db, _InputIterator first, _InputIterator last) :
	_db(db)
    {
	while (first != last)
	{
	    insert(*first);
	    ++first;
	}
    }
#else
    DBMap(const DBPtr& db, const value_type* first, const value_type* last) :
	_db(db)
    {
	while (first != last)
	{
	    insert(*first);
	    ++first;
	}
    }
    DBMap(const DBPtr& db, const_iterator first, const_iterator last) :
	_db(db)
    { 
	while (first != last)
	{
	    insert(*first);
	    ++first;
	}
    }
#endif /*__STL_MEMBER_TEMPLATES */

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
	if (count() != rhs.count())
	    return false;

	for (const_iterator p = rhs.begin() ; p != rhs.end() ; ++p)
	{
	    const_iterator q = rhs.find(p->first);
	    if (q == rhs.end())
	    {
		return false;
	    }
	    if (p->second != q->second)
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
	DBPtr tmp = _db;
	_db = rhs._db;
	rhs._db = tmp;
    }

    iterator begin()
    {
	try
	{
	    return iterator(_db, _db->getCursor());
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
	    return const_iterator(_db, _db->getCursor());
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
	return (size_type)_db->getNumberOfRecords();
    }

    size_type max_size() const
    {
	return 0xffffffff; // TODO: is this the max?
    }

    mapped_type_reference operator[](const key_type& key)
    {
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());
	Freeze::Key k = KeyCodec::write(key, instance);
	mapped_type value;

	try
	{
	    Freeze::Value v = _db->get(k);
	    ValueCodec::read(value, v, instance);
	}
	catch(const DBNotFoundException&)
	{
	    value = mapped_type();
	    Freeze::Value v = ValueCodec::write(value, instance);
	    _db->put(k, v);
	}
	return mapped_type_reference(_db, key, value);
	
    }

    //
    // This method isn't in the STLport library - but it's referenced
    // in "STL Tutorial and Refrence Guide, Second Edition". It's not
    // currently implemented.
    //
    //const mapped_type& operator[](const key_type& key) const;

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
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());

	Freeze::Key k;
	Freeze::Value v;
	k = KeyCodec::write(key.first, instance);
	v = ValueCodec::write(key.second, instance);

	_db->put(k, v);
	DBCursorPtr cursor = _db->getCursorAtKey(k);

	return iterator(_db, cursor);
    }

    std::pair<iterator, bool> insert(const value_type& key)
    {
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());

	Freeze::Key k;
	Freeze::Value v;
	k = KeyCodec::write(key.first, instance);
	v = ValueCodec::write(key.second, instance);

	DBCursorPtr cursor;
	bool inserted;

	try
	{
	    //
	    // Does the value exist already?
	    //
	    cursor = _db->getCursorAtKey(k);
	    inserted = false;
	}
	catch(const DBNotFoundException&)
	{
	    inserted = true;
	}

	_db->put(k, v);
	if (inserted)
	{
	    cursor = _db->getCursorAtKey(k);
	}
	return std::pair<iterator, bool>(iterator(_db, cursor), inserted);
    }

#ifdef __STL_MEMBER_TEMPLATES
    template <typename InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
	while (first != last)
	{
	    insert(*first);
	    ++first;
	}
    }
#else
    void insert(const value_type* first, const value_type* last)
    {
	while (first != last)
	{
	    insert(*first);
	    ++first;
	}
    }
    void insert(const_iterator first, const_iterator last)
    {
	while (first != last)
	{
	    insert(*first);
	    ++first;
	}
    }
#endif

    void erase(iterator position)
    {
	position._cursor->del();
    }

    size_type erase(const key_type& key)
    {
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());

	Freeze::Key k = KeyCodec::write(key, instance);

	try
	{
	    _db->del(k);
	}
	catch(const DBNotFoundException&)
	{
	    return 0;
	}

	return 1;
    }

    void erase(iterator first, iterator last)
    {
	while (first != last)
	{
	    first._cursor->del();
	    ++first;
	}
    }

    void clear()
    {
	_db->clear();
    }

    iterator find(const key_type& key)
    {
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());

	Freeze::Key k;
	k = KeyCodec::write(key, instance);

	try
	{
	    DBCursorPtr cursor = _db->getCursorAtKey(k);
	    return iterator(_db, cursor);
	}
	catch(const DBNotFoundException&)
	{
	    //
	    // The record doesn't exist, return the end() iterator.
	    //
	}
	return end();
    }

    const_iterator find(const key_type& key) const
    {
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());

	Freeze::Key k;
	k = KeyCodec::write(key, instance);

	try
	{
	    DBCursorPtr cursor = _db->getCursorAtKey(k);
	    return const_iterator(_db, cursor);
	}
	catch(const DBNotFoundException&)
	{
	    //
	    // The record doesn't exist, return the end() iterator.
	    //
	}
	return end();
    }

    size_type count(const key_type& key) const
    {
	if (find(key) != end())
	    return 1;
	return 0;
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

    DBPtr _db;
};

} // End namespace Freeze

//
// This is for MSVC.
//
# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
namespace std
{

template <class key_type, class mapped_type, class KeyCodec, class ValueCodec>
inline pair<key_type, mapped_type>*
value_type(const Freeze::DBIterator<key_type, mapped_type, KeyCodec, ValueCodec>&)
{
    return (pair<key_type, mapped_type>*)0;
}

template <class key_type, class mapped_type, class KeyCodec, class ValueCodec>
inline pair<key_type, mapped_type>*
value_type(const Freeze::ConstDBIterator<key_type, mapped_type, KeyCodec, ValueCodec>&)
{
    return (pair<key_type, mapped_type>*)0;
}

inline forward_iterator_tag iterator_category(const Freeze::DBIteratorBase&)
{
    return forward_iterator_tag();
}

inline ptrdiff_t* distance_type(const Freeze::DBIteratorBase&) { return (ptrdiff_t*) 0; }

} // End namespace std

#endif /* _STLP_CLASS_PARTIAL_SPECIALIZATION */

#endif
