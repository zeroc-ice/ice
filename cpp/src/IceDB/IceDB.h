// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_DB_H
#define ICE_DB_H

#include <IceUtil/Exception.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Initialize.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

#include <lmdb.h>

#ifndef ICE_DB_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_DB_API /**/
#   elif defined(ICE_DB_API_EXPORTS)
#       define ICE_DB_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_DB_API ICE_DECLSPEC_IMPORT
#   endif
#endif

//
// Automatically link IceDB37[D|++11|++11D].lib with Visual C++
//
#if !defined(ICE_BUILDING_ICE_DB) && defined(ICE_DB_API_EXPORTS)
#   define ICE_BUILDING_ICE_DB
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_DB)
#   pragma comment(lib, ICE_LIBNAME("IceDB"))
#endif

namespace IceDB
{

const size_t maxKeySize = 511;

//
// LMDBException wraps an error condition (and error code)
// returned by LMDB
//
class ICE_DB_API LMDBException : public IceUtil::Exception
{
public:

    LMDBException(const char*, int, int);
#ifndef ICE_CPP11_COMPILER
    virtual ~LMDBException() throw();
#endif

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual LMDBException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    int error() const;

private:

    const int _error;
    static const char* _name;
};

//
// KeyTooLongException is thrown if we attempt to marshal a
// key with a marshaled representation longer than maxKeySize.
//
class ICE_DB_API KeyTooLongException : public IceUtil::Exception
{
public:

    KeyTooLongException(const char*, int, size_t);
#ifndef ICE_CPP11_COMPILER
    virtual ~KeyTooLongException() throw();
#endif

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual KeyTooLongException* ice_clone() const;
#endif
    virtual void ice_throw() const;

private:

    const size_t _size;
    static const char* _name;
};

//
// The creation of an Env fails with BadEnvException when this
// Env's max key size is smaller than maxKeySize.
//
class ICE_DB_API BadEnvException : public IceUtil::Exception
{
public:

    BadEnvException(const char*, int, size_t);
#ifndef ICE_CPP11_COMPILER
    virtual ~BadEnvException() throw();
#endif

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual BadEnvException* ice_clone() const;
#endif
    virtual void ice_throw() const;

private:

    const size_t _size;
    static const char* _name;
};

//
// Codec reads and writes T to/from MDB_val
//
// Partial specializations of this template must provide:
//
// Read [out] T from [in] MDB_val using [in] context C
// static void read(T& , const MDB_val&, const C&);
//
// Write [in] T into [out] MDB_val using [in] context C
// [out] H& holds the memory for MDB_val.mv_data.
// static void write(const T&, MDB_val&, H&, const C&);
//
// Write [in] T into [in/out] MDB_val using [in] context C
// [in] MDB_val contains the destination array
// write returns true on success, and false if the provided
// array is too small.
// On failure, MDB_val.mv_size contains the marshaled key
// size if known, and 0 if not know.
// static bool write(const T&, MDB_val&, H&, const C&);
//
template<typename T, typename C, typename H>
struct Codec;

class ICE_DB_API Env
{
public:

    explicit Env(const std::string&, MDB_dbi = 0, size_t = 0, unsigned int = 0);
    ~Env();

    void close();

    MDB_env* menv() const;

private:

    // Not implemented: class is not copyable
    Env(const Env&);
    void operator=(const Env&);

    MDB_env* _menv;
};

class ICE_DB_API Txn
{
public:

    virtual ~Txn();

    void commit();
    void rollback();

    MDB_txn* mtxn() const;

protected:

    explicit Txn(const Env&, unsigned int);

    MDB_txn* _mtxn;

private:

    // Not implemented: class is not copyable
    Txn(const Txn&);
    void operator=(const Txn&);
};

class ICE_DB_API ReadOnlyTxn : public Txn
{
public:

    virtual ~ReadOnlyTxn();

    explicit ReadOnlyTxn(const Env&);

    void reset();
    void renew();
};

class ICE_DB_API ReadWriteTxn : public Txn
{
public:

    virtual ~ReadWriteTxn();

    explicit ReadWriteTxn(const Env&);
};

class ICE_DB_API DbiBase
{
public:

    void clear(const ReadWriteTxn&);
    MDB_dbi mdbi() const;

    virtual ~DbiBase();

protected:

    DbiBase(const Txn&, const std::string&, unsigned int, MDB_cmp_func*);
    DbiBase();

    // default copy ctor and assignment operator are OK

    bool get(const Txn&, MDB_val*, MDB_val*) const;
    void put(const ReadWriteTxn&, MDB_val*, MDB_val*, unsigned int);
    bool find(const Txn&, MDB_val*) const;
    bool del(const ReadWriteTxn&, MDB_val*, MDB_val*);

private:

    MDB_dbi _mdbi;
};

template<typename K, typename D, typename C, typename H>
class Dbi : public DbiBase
{
public:

    Dbi(const Txn& txn, const std::string& name, const C& ctx, unsigned int flags = 0, MDB_cmp_func* cmp = 0) :
        DbiBase(txn, name, flags, cmp),
        _marshalingContext(ctx)
    {
    }

    Dbi()
    {
    }

    bool get(const Txn& txn, const K& key, D& data) const
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};

        if(Codec<K, C, H>::write(key, mkey, _marshalingContext))
        {
            MDB_val mdata;
            if(DbiBase::get(txn, &mkey, &mdata))
            {
                Codec<D, C, H>::read(data, mdata, _marshalingContext);
                return true;
            }
        }
        return false;
    }

    void put(const ReadWriteTxn& txn, const K& key, const D& data, unsigned int flags = 0)
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};

        if(Codec<K, C, H>::write(key, mkey, _marshalingContext))
        {
            H hdata;
            MDB_val mdata;
            Codec<D, C, H>::write(data, mdata, hdata, _marshalingContext);
            DbiBase::put(txn, &mkey, &mdata, flags);
        }
        else
        {
            throw KeyTooLongException(__FILE__, __LINE__, mkey.mv_size);
        }
    }

    bool find(const Txn& txn, const K& key) const
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};
        if(Codec<K, C, H>::write(key, mkey, _marshalingContext))
        {
            return DbiBase::find(txn, &mkey);
        }
        else
        {
            return false;
        }
    }

    bool del(const ReadWriteTxn& txn, const K& key, const D& data)
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};
        if(Codec<K, C, H>::write(key, mkey, _marshalingContext))
        {
            H hdata;
            MDB_val mdata;
            Codec<D, C, H>::write(data, mdata, hdata, _marshalingContext);
            return DbiBase::del(txn, &mkey, &mdata);
        }
        else
        {
            return false;
        }
    }

    bool del(const ReadWriteTxn& txn, const K& key)
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};
        if(Codec<K, C, H>::write(key, mkey, _marshalingContext))
        {
            return DbiBase::del(txn, &mkey, 0);
        }
        else
        {
            return false;
        }
    }

    C marshalingContext() const
    {
        return _marshalingContext;
    }

private:

    C _marshalingContext;
};

class ICE_DB_API CursorBase
{
public:

    void close();

    MDB_cursor* mcursor() const;

    virtual ~CursorBase();

protected:

    CursorBase(MDB_dbi dbi, const Txn& txn, bool);

    bool get(MDB_val*, MDB_val*, MDB_cursor_op);
    void put(MDB_val*, MDB_val*, unsigned int);
    bool find(MDB_val*);
    bool find(MDB_val*, MDB_val*);
    void del(unsigned int);
    void renew(const ReadOnlyTxn&);

private:

    // Not implemented: class is not copyable
    CursorBase(const CursorBase&);
    void operator=(const CursorBase&);

    MDB_cursor* _mcursor;
    const bool _readOnly;
};

template<typename K, typename D, typename C, typename H>
class Cursor : public CursorBase
{
public:

    Cursor(const Dbi<K, D, C, H>& dbi, const ReadOnlyTxn& txn) :
        CursorBase(dbi.mdbi(), txn, true),
        _marshalingContext(dbi.marshalingContext())
    {
    }

    Cursor(const Dbi<K, D, C, H>& dbi, const ReadWriteTxn& txn) :
        CursorBase(dbi.mdbi(), txn, false),
        _marshalingContext(dbi.marshalingContext())
    {
    }

    Cursor(const Dbi<K, D, C, H>& dbi, const Txn& txn) :
        CursorBase(dbi.mdbi(), txn, dynamic_cast<const ReadOnlyTxn*>(&txn) != 0),
        _marshalingContext(dbi.marshalingContext())
    {
    }

    bool get(K& key, D& data, MDB_cursor_op op)
    {
        MDB_val mkey, mdata;
        if(CursorBase::get(&mkey, &mdata, op))
        {
            Codec<K, C, H>::read(key, mkey, _marshalingContext);
            Codec<D, C, H>::read(data, mdata, _marshalingContext);
            return true;
        }
        return false;
    }

    bool find(const K& key)
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};
        if(Codec<K, C, H>::write(key, mkey, _marshalingContext))
        {
            return CursorBase::find(&mkey);
        }
        else
        {
            return false;
        }
    }

    bool find(const K& key, D& data)
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};
        if(Codec<K, C, H>::write(key, mkey, _marshalingContext))
        {
            MDB_val mdata;
            if(CursorBase::find(&mkey, &mdata))
            {
                Codec<D, C, H>::read(data, mdata, _marshalingContext);
                return true;
            }
        }
        return false;
    }

protected:

    C _marshalingContext;
};

template<typename K, typename D, typename C, typename H>
class ReadWriteCursor : public Cursor<K, D, C, H>
{
public:

    ReadWriteCursor(const Dbi<K, D, C, H>& dbi, const ReadWriteTxn& txn) :
        Cursor<K, D, C, H>(dbi, txn)
    {
    }

    void put(const K& key, const D& data, unsigned int flags = 0)
    {
        unsigned char kbuf[maxKeySize];
        MDB_val mkey = {maxKeySize, kbuf};
        if(Codec<K, C, H>::write(key, mkey, this->_marshalingContext))
        {
            H hdata;
            MDB_val mdata;
            Codec<D, C, H>::write(data, mdata, hdata, this->_marshalingContext);
            CursorBase::put(&mkey, &mdata, flags);
        }
        else
        {
            throw KeyTooLongException(__FILE__, __LINE__, mkey.mv_size);
        }
    }

    void del(unsigned int flags = 0)
    {
        CursorBase::del(flags);
    }
};

template<typename K, typename D, typename C, typename H>
class ReadOnlyCursor : public Cursor<K, D, C, H>
{
public:

    ReadOnlyCursor(const Dbi<K, D, C, H>& dbi, const ReadOnlyTxn& txn) :
        Cursor<K, D, C, H>(dbi, txn)
    {
    }

    void renew(const ReadOnlyTxn& txn)
    {
        CursorBase::renew(txn);
    }
};

//
// Partial specialization of Codec for Ice encoding
//
struct IceContext
{
    Ice::CommunicatorPtr communicator;
    Ice::EncodingVersion encoding;
};

template<typename T>
struct Codec<T, IceContext, Ice::OutputStream>
{
    static void read(T& t, const MDB_val& val, const IceContext& ctx)
    {
        std::pair<const Ice::Byte*, const Ice::Byte*> p(static_cast<const Ice::Byte*>(val.mv_data),
                                                        static_cast<const Ice::Byte*>(val.mv_data) + val.mv_size);
        Ice::InputStream in(ctx.communicator, ctx.encoding, p);
        in.read(t);
    }

    static void write(const T& t, MDB_val& val, Ice::OutputStream& holder, const IceContext& ctx)
    {
        holder.initialize(ctx.communicator, ctx.encoding);
        holder.write(t);
        val.mv_size = holder.b.size();
        val.mv_data = &holder.b[0];
    }

    static bool write(const T& t, MDB_val& val, const IceContext& ctx)
    {
        const size_t limit = val.mv_size;
        std::pair<Ice::Byte*, Ice::Byte*> p(reinterpret_cast<Ice::Byte*>(val.mv_data),
                                            reinterpret_cast<Ice::Byte*>(val.mv_data) + limit);
        Ice::OutputStream stream(ctx.communicator, ctx.encoding, p);
        stream.write(t);
        val.mv_size = stream.b.size();
        return stream.b.size() > limit ? false : true;
    }
};

//
// Returns computed mapSize in bytes.
// When the input parameter is <= 0, returns a platform-dependent default
// (currently 0 on Windows and 100 MB on other platforms).
// Otherwise, returns input parameter * 1 MB.
//

ICE_DB_API size_t getMapSize(int);

}

#endif
