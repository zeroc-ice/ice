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

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

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
    value_type& operator*() const
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
	return _ref;
    }

    value_type* operator->() { return &(operator*()); }

    //
    // This special method allows writing back into the database.
    //
    void set(const mapped_type& value)
    {
	IceInternal::InstancePtr instance = IceInternal::getInstance(_db->getCommunicator());
	Freeze::Value v = ValueCodec::write(value, instance);
	_cursor->set(v);
    }

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
    mutable value_type _ref;
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

    //
    // Note that this doesn't follow the regular iterator mapping:
    //
    // value_type operator*() const
    //
    value_type& operator*() const
    {
	key_type key;
	mapped_type value;

	//
	// !IMPORTANT!
	//
	// This method has to cache the returned value to implement
	// operator->().
	//
	const_cast<key_type&>(_ref.first) = key;
	const_cast<mapped_type&>(_ref.second) = value;
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

//XXX update
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

} // End namespace std

#endif /* _STLP_CLASS_PARTIAL_SPECIALIZATION */

#endif
