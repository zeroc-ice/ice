// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
class Dbt;

namespace Freeze
{

class IteratorHelper;
class MapHelper;

class MapIndexI;
class MapHelperI;
class IteratorHelperI;
class SharedDb;

class FREEZE_API KeyCompareBase : public IceUtil::Shared
{
public:

    KeyCompareBase(bool);

    bool compareEnabled() const;

    virtual int compare(const Key&, const Key&) = 0;

private:

    const bool _enabled;
};
typedef IceUtil::Handle<KeyCompareBase> KeyCompareBasePtr;

class FREEZE_API MapIndexBase : public KeyCompareBase
{
public:

    virtual ~MapIndexBase();

    const std::string& name() const;

    IteratorHelper* begin(bool) const;
    IteratorHelper* untypedFind(const Key&, bool, bool) const;
    IteratorHelper* untypedLowerBound(const Key&, bool) const;
    IteratorHelper* untypedUpperBound(const Key&, bool) const;

    int untypedCount(const Key&) const;

    //
    // Implemented by the generated code
    //
    virtual void marshalKey(const Value&, Key&) const = 0;

protected:

    MapIndexBase(const std::string&, bool);

    Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;

private:

    friend class MapHelperI;
    friend class IteratorHelperI;
    friend class SharedDb;
    friend class MapDb;

    std::string _name;
    MapIndexI* _impl;
    const MapHelperI* _map;
};
typedef IceUtil::Handle<MapIndexBase> MapIndexBasePtr;

class FREEZE_API MapHelper
{
public:

    static MapHelper*
    create(const ConnectionPtr& connection,
           const std::string& dbName,
           const std::string& key,
           const std::string& value,
           const KeyCompareBasePtr&,
           const std::vector<MapIndexBasePtr>&,
           bool createDb);

    static void
    recreate(const ConnectionPtr& connection,
             const std::string& dbName,
             const std::string& key,
             const std::string& value,
             const KeyCompareBasePtr&,
             const std::vector<MapIndexBasePtr>&);

    virtual ~MapHelper() = 0;

    virtual IteratorHelper*
    find(const Key&, bool) const = 0;

    virtual IteratorHelper*
    find(const Dbt&, bool) const = 0;

    virtual IteratorHelper*
    lowerBound(const Key&, bool) const = 0;

    virtual IteratorHelper*
    upperBound(const Key&, bool) const = 0;

    virtual void
    put(const Key&, const Value&) = 0;

    virtual void
    put(const Dbt&, const Dbt&) = 0;

    virtual size_t
    erase(const Key&) = 0;

    virtual size_t
    erase(const Dbt&) = 0;

    virtual size_t
    count(const Key&) const = 0;

    virtual size_t
    count(const Dbt&) const = 0;

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

    virtual void
    closeDb() = 0;

    virtual ConnectionPtr
    getConnection() const = 0;
};

class FREEZE_API IteratorHelper
{
public:

    static IteratorHelper*
    create(const MapHelper& m, bool readOnly);

    virtual
    ~IteratorHelper() ICE_NOEXCEPT_FALSE = 0;

    virtual IteratorHelper*
    clone() const = 0;

    virtual const Key*
    get() const = 0;

    virtual void
    get(const Key*&, const Value*&) const = 0;

    virtual void
    set(const Value&) = 0;

    virtual void
    set(const Dbt&) = 0;

    virtual void
    erase() = 0;

    virtual bool
    next() const = 0;
};

//
// Forward declaration
//
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec, typename Compare>
class Map;
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec, typename Compare>
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
// restriction that it's only possible to explicitly write back into
// the database.
//
// Two iterators are equal if they use the same database and their
// current records have the same key.
//
// TODO: It's possible to implement bidirectional iterators, if
// necessary.
//
template<typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec, typename Compare>
class Iterator : public IteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    Iterator(IteratorHelper* helper, const Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding) :
        _helper(helper),
        _communicator(communicator),
        _encoding(encoding),
        _refValid(false)
    {
    }

    Iterator() :
        _encoding(Ice::Encoding_1_0),
        _refValid(false)
    {
    }

    Iterator(const Iterator& rhs) :
        _communicator(rhs._communicator),
        _encoding(rhs._encoding),
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
            _encoding = rhs._encoding;
            _refValid = false;
        }

        return *this;
    }

    ~Iterator() ICE_NOEXCEPT_FALSE
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

        ValueCodec v(value, _communicator, _encoding);
        _helper->set(v.dbt());
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

        KeyCodec::read(key, *k, _communicator, _encoding);
        ValueCodec::read(value, *v, _communicator, _encoding);
    }

    friend class ConstIterator<key_type, mapped_type, KeyCodec, ValueCodec, Compare>;
    friend class Map<key_type, mapped_type, KeyCodec, ValueCodec, Compare>;

    IceUtil::UniquePtr<IteratorHelper> _helper;
    Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;

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
template <typename key_type, typename mapped_type, typename KeyCodec, typename ValueCodec, typename Compare>
class ConstIterator : public IteratorBase
{
public:

    typedef ptrdiff_t difference_type;

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef value_type* pointer;

    typedef value_type& reference;

    ConstIterator(IteratorHelper* helper,
                  const Ice::CommunicatorPtr& communicator,
                  const Ice::EncodingVersion& encoding) :
        _helper(helper),
        _communicator(communicator),
        _encoding(encoding),
        _refValid(false)
    {
    }

    ConstIterator() :
        _encoding(Ice::Encoding_1_0),
        _refValid(false)
    {
    }

    ConstIterator(const ConstIterator& rhs) :
        _communicator(rhs._communicator),
        _encoding(rhs._encoding),
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
    ConstIterator(const Iterator<key_type, mapped_type, KeyCodec, ValueCodec, Compare>& rhs) :
        _communicator(rhs._communicator),
        _encoding(rhs._encoding),
        _refValid(false)
    {
        if(rhs._helper.get() != 0)
        {
            _helper.reset(rhs._helper->clone());
        }
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
            _encoding = rhs._encoding;
            _refValid = false;
        }

        return *this;
    }

    //
    // Create const_iterator from iterator.
    //
    ConstIterator& operator=(const Iterator<key_type, mapped_type, KeyCodec, ValueCodec, Compare>& rhs)
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
        _encoding = rhs._encoding;
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

        KeyCodec::read(key, *k, _communicator, _encoding);
        ValueCodec::read(value, *v, _communicator, _encoding);
    }

    friend class Map<key_type, mapped_type, KeyCodec, ValueCodec, Compare>;

    IceUtil::UniquePtr<IteratorHelper> _helper;
    Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;

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

struct IceEncodingCompare {};

template<typename key_type, typename KeyCodec, typename Compare>
class KeyCompare : public KeyCompareBase
{
public:

    KeyCompare(const Compare& mapCompare,
               const Ice::CommunicatorPtr& communicator,
               const Ice::EncodingVersion& encoding) :
        KeyCompareBase(true),
        _compare(mapCompare),
        _communicator(communicator),
        _encoding(encoding)
    {}

    virtual int compare(const Key& dbKey1, const Key& dbKey2)
    {
        key_type key1;
        KeyCodec::read(key1, dbKey1, _communicator, _encoding);
        key_type key2;
        KeyCodec::read(key2, dbKey2, _communicator, _encoding);

        if(_compare(key1, key2))
        {
            return -1;
        }
        else if(_compare(key2, key1))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

private:

    Compare _compare;
    const Ice::CommunicatorPtr _communicator;
    const Ice::EncodingVersion _encoding;
};

//
// Partial template specialization:
// do nothing for the IceEncodingCompare comparator
//
template<typename key_type, typename KeyCodec>
class KeyCompare<key_type, KeyCodec, IceEncodingCompare> : public KeyCompareBase
{
public:

    KeyCompare(const IceEncodingCompare&, const Ice::CommunicatorPtr&, const Ice::EncodingVersion&):
        KeyCompareBase(false)
    {}

    virtual int compare(const Key&, const Key&)
    {
        assert(0);
        return 0;
    }
};

//
// Need to separate MapIndex template class because _communicator is
// set later
//
template<typename key_type, typename KeyCodec, typename Compare>
class MapIndex : public MapIndexBase
{
public:

    virtual int compare(const Key& dbKey1, const Key& dbKey2)
    {
        key_type key1;
        KeyCodec::read(key1, dbKey1, _communicator, _encoding);
        key_type key2;
        KeyCodec::read(key2, dbKey2, _communicator, _encoding);

        if(_compare(key1, key2))
        {
            return -1;
        }
        else if(_compare(key2, key1))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

protected:

    MapIndex(const std::string& mapName, const Compare& mapCompare) :
        MapIndexBase(mapName, true),
        _compare(mapCompare)
    {}

private:

    Compare _compare;
};

//
// Partial template specialization:
// do nothing for the IceEncodingCompare comparator
//
template<typename key_type, typename KeyCodec>
class MapIndex<key_type, KeyCodec, IceEncodingCompare> : public MapIndexBase
{
public:

    virtual int compare(const Key&, const Key&)
    {
        assert(0);
        return 0;
    }

protected:

    MapIndex(const std::string& mapName, const IceEncodingCompare&):
        MapIndexBase(mapName, false)
    {}
};

class FREEZE_API MapCodecBase
{
public:

    const Dbt& dbt() const { return *_dbt; }

protected:

    MapCodecBase(const Ice::CommunicatorPtr&, const Ice::EncodingVersion&);
    ~MapCodecBase();

    void init();

    IceInternal::BasicStream _stream;
    Dbt* _dbt;
};

//
// Codec template for all key types. Marshaled keys are NOT encapsulated.
//
template<typename T>
class MapKeyCodec : public MapCodecBase
{
public:

    //
    // Use the constructor to marshal a value while avoiding unnecessary copies of the
    // marshaled data. The inherited dbt() accessor provides a Dbt value initialized with
    // the marshaled bytes.
    //
    MapKeyCodec(const T& v, const Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding) :
        MapCodecBase(communicator, encoding)
    {
        _stream.write(v);
        init();
    }

    template<typename U> static void write(const U& v, std::vector<Ice::Byte>& bytes,
                                           const Ice::CommunicatorPtr& communicator,
                                           const Ice::EncodingVersion& encoding)
    {
        IceInternal::BasicStream stream(IceInternal::getInstance(communicator).get(), encoding);
        stream.write(v);
        std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
    }

    template<typename U> static void read(U& v, const std::vector<Ice::Byte>& bytes,
                                          const Ice::CommunicatorPtr& communicator,
                                          const Ice::EncodingVersion& encoding)
    {
        IceInternal::BasicStream stream(IceInternal::getInstance(communicator).get(), encoding, &bytes[0],
                                        &bytes[0] + bytes.size());
        stream.read(v);
    }
};

//
// Codec template for all value types except those that use classes. Marshaled values are encapsulated.
//
template<typename T>
class MapValueCodec : public MapCodecBase
{
public:

    //
    // Use the constructor to marshal a value while avoiding unnecessary copies of the
    // marshaled data. The inherited dbt() accessor provides a Dbt value initialized with
    // the marshaled bytes.
    //
    MapValueCodec(const T& v, const Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding) :
        MapCodecBase(communicator, encoding)
    {
        _stream.startWriteEncaps();
        _stream.write(v);
        _stream.endWriteEncaps();
        init();
    }

    template<typename U> static void write(const U& v, std::vector<Ice::Byte>& bytes,
                                           const Ice::CommunicatorPtr& communicator,
                                           const Ice::EncodingVersion& encoding)
    {
        IceInternal::BasicStream stream(IceInternal::getInstance(communicator).get(), encoding);
        stream.startWriteEncaps();
        stream.write(v);
        stream.endWriteEncaps();
        std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
    }

    template<typename U> static void read(U& v, const std::vector<Ice::Byte>& bytes,
                                          const Ice::CommunicatorPtr& communicator,
                                          const Ice::EncodingVersion& encoding)
    {
        IceInternal::BasicStream stream(IceInternal::getInstance(communicator).get(), encoding, &bytes[0],
                                        &bytes[0] + bytes.size());
        stream.startReadEncaps();
        stream.read(v);
        stream.endReadEncaps();
    }
};

//
// Codec template for all value types that use classes. Marshaled values are encapsulated.
//
template<typename T>
class MapObjectValueCodec : public MapCodecBase
{
public:

    //
    // Use the constructor to marshal a value while avoiding unnecessary copies of the
    // marshaled data. The inherited dbt() accessor provides a Dbt value initialized with
    // the marshaled bytes.
    //
    MapObjectValueCodec(const T& v, const Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding) :
        MapCodecBase(communicator, encoding)
    {
        _stream.startWriteEncaps();
        _stream.write(v);
        _stream.writePendingObjects();
        _stream.endWriteEncaps();
        init();
    }

    template<typename U> static void write(const U& v, std::vector<Ice::Byte>& bytes,
                                           const Ice::CommunicatorPtr& communicator,
                                           const Ice::EncodingVersion& encoding)
    {
        IceInternal::BasicStream stream(IceInternal::getInstance(communicator).get(), encoding);
        stream.startWriteEncaps();
        stream.write(v);
        stream.writePendingObjects();
        stream.endWriteEncaps();
        std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
    }

    template<typename U> static void read(U& v, const std::vector<Ice::Byte>& bytes,
                                          const Ice::CommunicatorPtr& communicator,
                                          const Ice::EncodingVersion& encoding)
    {
        IceInternal::BasicStream stream(IceInternal::getInstance(communicator).get(), encoding, &bytes[0],
                                        &bytes[0] + bytes.size());
        stream.sliceObjects(false);
        stream.startReadEncaps();
        stream.read(v);
        stream.readPendingObjects();
        stream.endReadEncaps();
    }
};

//
// A sorted map, similar to a std::map, with one notable difference:
// operator[] is not provided.
//
//
// TODO: implement bidirectional iterators.
//
template<typename key_type, typename mapped_type,
         typename KeyCodec, typename ValueCodec,
         typename Compare = IceEncodingCompare>
class Map
{
public:

    typedef std::pair<const key_type, const mapped_type> value_type;

    typedef Iterator<key_type, mapped_type, KeyCodec, ValueCodec, Compare> iterator;
    typedef ConstIterator<key_type, mapped_type, KeyCodec, ValueCodec, Compare> const_iterator;

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
        const std::string& keyTypeId,
        const std::string& valueTypeId,
        bool createDb = true,
        const Compare& compare = Compare()) :
        _communicator(connection->getCommunicator()),
        _encoding(connection->getEncoding())
    {
        KeyCompareBasePtr keyCompare = new KeyCompare<key_type, KeyCodec, Compare>(compare, _communicator, _encoding);
        std::vector<MapIndexBasePtr> indices;

        _helper.reset(MapHelper::create(connection, dbName, keyTypeId, valueTypeId, keyCompare, indices, createDb));
    }

    template<class _InputIterator>
    Map(const Freeze::ConnectionPtr& connection,
        const std::string& dbName,
        const std::string& keyTypeId,
        const std::string& valueTypeId,
        bool createDb,
        _InputIterator first, _InputIterator last,
        const Compare& compare = Compare()) :
        _communicator(connection->getCommunicator()),
        _encoding(connection->getEncoding())
    {
        KeyCompareBasePtr keyCompare = new KeyCompare<key_type, KeyCodec, Compare>(compare, _communicator, _encoding);

        std::vector<MapIndexBasePtr> indices;

        _helper.reset(MapHelper::create(connection, dbName, keyTypeId, valueTypeId, keyCompare, indices, createDb));

        while(first != last)
        {
            put(*first);
            ++first;
        }
    }

    ~Map()
    {
    }

    // static void recreate(const Freeze::ConnectionPtr& connection,
    //                      const std::string& dbName,
    //                      const Compare& compare = Compare())
    // {
    //     KeyCompareBasePtr keyCompare = new KeyCompare<key_type, KeyCodec, Compare>(compare,
    //                                                                                connection->getCommunicator(),
    //                                                                                connection->getEncoding());

    //     std::vector<MapIndexBasePtr> indices;
    //     MapHelper::recreate(connection, dbName, KeyCodec::typeId(), ValueCodec::typeId(), keyCompare, indices);
    // }


    bool operator==(const Map& rhs) const
    {
        //
        // This does a memberwise equality for the entire contents of
        // the database. While slow this is always correct. Database
        // equality is not necessarily correct in the context of a
        // transaction.
        //
        if(size() != rhs.size())
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
        Ice::EncodingVersion tmpEnc = _encoding;
        _communicator = rhs._communicator;
        _encoding = rhs._encoding;
        rhs._communicator = tmpCom;
        rhs._encoding = tmpEnc;
    }

    iterator begin()
    {
        try
        {
            return iterator(IteratorHelper::create(*_helper.get(), false), _communicator, _encoding);
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
            return const_iterator(IteratorHelper::create(*_helper.get(), true), _communicator, _encoding);
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
    // in "STL Tutorial and Reference Guide, Second Edition". It's not
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
        KeyCodec k(key.first, _communicator, _encoding);

        iterator r = iterator(_helper->find(k.dbt(), false), _communicator, _encoding);

        if(r == end())
        {
            ValueCodec v(key.second, _communicator, _encoding);

            _helper->put(k.dbt(), v.dbt());
            r = iterator(_helper->find(k.dbt(), false), _communicator, _encoding);
        }

        return r;
    }

    std::pair<iterator, bool> insert(const value_type& key)
    {
        KeyCodec k(key.first, _communicator, _encoding);

        iterator r = iterator(_helper->find(k.dbt(), false), _communicator, _encoding);
        bool inserted = false;

        if(r == end())
        {
            ValueCodec v(key.second, _communicator, _encoding);

            _helper->put(k.dbt(), v.dbt());
            inserted = true;
            r = iterator(_helper->find(k.dbt(), false), _communicator, _encoding);
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
        KeyCodec k(key.first, _communicator, _encoding);
        ValueCodec v(key.second, _communicator, _encoding);

        _helper->put(k.dbt(), v.dbt());
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
        KeyCodec k(key, _communicator, _encoding);

        return _helper->erase(k.dbt());
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
    // destroy is not a standard function
    //
    void destroy()
    {
        _helper->destroy();
    }

    //
    // closeDb closes the underlying Berkeley DB database
    //
    void closeDb()
    {
        _helper->closeDb();
    }

    iterator find(const key_type& key)
    {
        KeyCodec k(key, _communicator, _encoding);

        return iterator(_helper->find(k.dbt(), false), _communicator, _encoding);
    }

    const_iterator find(const key_type& key) const
    {
        KeyCodec k(key, _communicator, _encoding);

        return const_iterator(_helper->find(k.dbt(), true), _communicator, _encoding);
    }

    size_type count(const key_type& key) const
    {
        KeyCodec k(key, _communicator, _encoding);

        return _helper->count(k.dbt());
    }

    iterator lower_bound(const key_type& key)
    {
        Key k;
        KeyCodec::write(key, k, _communicator, _encoding);

        return iterator(_helper->lowerBound(k, false), _communicator, _encoding);
    }

    const_iterator lower_bound(const key_type& key) const
    {
        Key k;
        KeyCodec::write(key, k, _communicator, _encoding);

        return iterator(_helper->lowerBound(k, true), _communicator, _encoding);
    }

    iterator upper_bound(const key_type& key)
    {
        Key k;
        KeyCodec::write(key, k, _communicator, _encoding);

        return iterator(_helper->upperBound(k, false), _communicator, _encoding);
    }

    const_iterator upper_bound(const key_type& key) const
    {
        Key k;
        KeyCodec::write(key, k, _communicator, _encoding);

        return iterator(_helper->upperBound(k, true), _communicator, _encoding);
    }

    std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        return std::make_pair(lower_bound(key), upper_bound(key));
    }

    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        return std::make_pair(lower_bound(key), upper_bound(key));
    }

    const Ice::CommunicatorPtr& communicator() const
    {
        return _communicator;
    }

    //
    // getConnection returns the associated connection
    //
    ConnectionPtr getConnection() const
    {
        return _helper->getConnection();
    }

protected:

    Map(const Ice::CommunicatorPtr& mapCommunicator, const Ice::EncodingVersion& encoding) :
        _communicator(mapCommunicator),
        _encoding(encoding)
    {
    }

    IceUtil::UniquePtr<MapHelper> _helper;
    Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;
};

}

#endif
