// Copyright (c) ZeroC, Inc.

#ifndef ICE_DB_H
#define ICE_DB_H

#include "../Ice/FileUtil.h"
#include "Ice/Initialize.h"
#include "Ice/InputStream.h"
#include "Ice/LocalException.h"
#include "Ice/OutputStream.h"

#include <lmdb.h>

namespace IceDB
{
    const size_t maxKeySize = 511;

    //
    // LMDBException wraps an error condition (and error code)
    // returned by LMDB
    //
    class LMDBException final : public Ice::LocalException
    {
    public:
        LMDBException(const char* file, int line, int error);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // KeyTooLongException is thrown if we attempt to marshal a
    // key with a marshaled representation longer than maxKeySize.
    //
    class KeyTooLongException final : public Ice::LocalException
    {
    public:
        KeyTooLongException(const char* file, int line, size_t size);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // The creation of an Env fails with BadEnvException when this
    // Env's max key size is smaller than maxKeySize.
    //
    class BadEnvException final : public Ice::LocalException
    {
    public:
        BadEnvException(const char*, int, size_t);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // Codec reads and writes T to/from MDB_val
    //
    // Partial specializations of this template must provide:
    //
    // Read [out] T from [in] MDB_val using [in] context C
    // static void read(T& , const MDB_val&, const C&);
    //
    // Write [in] T into [out] MDB_val
    // [out] H& holds the memory for MDB_val.mv_data.
    // static void write(const T&, MDB_val&, H&);
    //
    // Write [in] T into [in/out] MDB_val
    // [in] MDB_val contains the destination array
    // write returns true on success, and false if the provided
    // array is too small.
    // On failure, MDB_val.mv_size contains the marshaled key
    // size if known, and 0 if not know.
    // static bool write(const T&, MDB_val&, H&);
    //
    template<typename T, typename C, typename H> struct Codec;

    class Env
    {
    public:
        explicit Env(const std::string&, MDB_dbi = 0, size_t = 0, unsigned int = 0);
        Env(const Env&) = delete;
        ~Env();

        Env& operator=(const Env&) = delete;

        void close();

        [[nodiscard]] MDB_env* menv() const;

    private:
        MDB_env* _menv;
    };

    class Txn
    {
    public:
        Txn(const Txn&) = delete;
        Txn& operator=(const Txn&) = delete;

        void commit();
        void rollback();

        [[nodiscard]] MDB_txn* mtxn() const;

        [[nodiscard]] bool isReadOnly() const { return _readOnly; }

    protected:
        Txn(const Env&, unsigned int);
        ~Txn();

        MDB_txn* _mtxn{nullptr};
        const bool _readOnly;
    };

    class ReadOnlyTxn : public Txn
    {
    public:
        explicit ReadOnlyTxn(const Env&);
        ~ReadOnlyTxn();
    };

    class ReadWriteTxn : public Txn
    {
    public:
        explicit ReadWriteTxn(const Env&);
        ~ReadWriteTxn();
    };

    class DbiBase
    {
    public:
        void clear(const ReadWriteTxn&);
        [[nodiscard]] MDB_dbi mdbi() const;

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

    template<typename K, typename D, typename C, typename H> class Dbi : public DbiBase
    {
    public:
        Dbi(const Txn& txn, const std::string& name, C ctx, unsigned int flags = 0, MDB_cmp_func* cmp = nullptr)
            : DbiBase(txn, name, flags, cmp),
              _marshalingContext(std::move(ctx))
        {
        }

        Dbi() = default;

        bool get(const Txn& txn, const K& key, D& data) const
        {
            unsigned char kbuf[maxKeySize];
            MDB_val mkey = {maxKeySize, kbuf};

            if (Codec<K, C, H>::write(key, mkey))
            {
                MDB_val mdata;
                if (DbiBase::get(txn, &mkey, &mdata))
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

            if (Codec<K, C, H>::write(key, mkey))
            {
                H hdata;
                MDB_val mdata;
                Codec<D, C, H>::write(data, mdata, hdata);
                DbiBase::put(txn, &mkey, &mdata, flags);
            }
            else
            {
                throw KeyTooLongException(__FILE__, __LINE__, mkey.mv_size);
            }
        }

        [[nodiscard]] bool find(const Txn& txn, const K& key) const
        {
            unsigned char kbuf[maxKeySize];
            MDB_val mkey = {maxKeySize, kbuf};
            if (Codec<K, C, H>::write(key, mkey))
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
            if (Codec<K, C, H>::write(key, mkey))
            {
                H hdata;
                MDB_val mdata;
                Codec<D, C, H>::write(data, mdata, hdata);
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
            if (Codec<K, C, H>::write(key, mkey))
            {
                return DbiBase::del(txn, &mkey, nullptr);
            }
            else
            {
                return false;
            }
        }

        [[nodiscard]] C marshalingContext() const { return _marshalingContext; }

    private:
        C _marshalingContext;
    };

    class CursorBase
    {
    public:
        void close();

        CursorBase(const CursorBase&) = delete;
        CursorBase& operator=(const CursorBase&) = delete;

    protected:
        CursorBase(MDB_dbi dbi, const Txn& txn);
        ~CursorBase();

        bool get(MDB_val*, MDB_val*, MDB_cursor_op);
        bool find(MDB_val*);
        bool find(MDB_val*, MDB_val*);

    private:
        MDB_cursor* _mcursor;
        const bool _readOnly;
    };

    template<typename K, typename D, typename C, typename H> class Cursor : public CursorBase
    {
    public:
        Cursor(const Dbi<K, D, C, H>& dbi, const Txn& txn)
            : CursorBase(dbi.mdbi(), txn),
              _marshalingContext(dbi.marshalingContext())
        {
        }

        bool get(K& key, D& data, MDB_cursor_op op)
        {
            MDB_val mkey, mdata;
            if (CursorBase::get(&mkey, &mdata, op))
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
            if (Codec<K, C, H>::write(key, mkey))
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
            if (Codec<K, C, H>::write(key, mkey))
            {
                MDB_val mdata;
                if (CursorBase::find(&mkey, &mdata))
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

    template<typename K, typename D, typename C, typename H> class ReadWriteCursor : public Cursor<K, D, C, H>
    {
    public:
        ReadWriteCursor(const Dbi<K, D, C, H>& dbi, const ReadWriteTxn& txn) : Cursor<K, D, C, H>(dbi, txn) {}
    };

    template<typename K, typename D, typename C, typename H> class ReadOnlyCursor : public Cursor<K, D, C, H>
    {
    public:
        ReadOnlyCursor(const Dbi<K, D, C, H>& dbi, const ReadOnlyTxn& txn) : Cursor<K, D, C, H>(dbi, txn) {}
    };

    //
    // Partial specialization of Codec for Ice encoding
    //
    struct IceContext
    {
        Ice::CommunicatorPtr communicator;
    };

    template<typename T> struct Codec<T, IceContext, Ice::OutputStream>
    {
        static void read(T& t, const MDB_val& val, const IceContext& ctx)
        {
            std::pair<const std::byte*, const std::byte*> p(
                static_cast<const std::byte*>(val.mv_data),
                static_cast<const std::byte*>(val.mv_data) + val.mv_size);
            Ice::InputStream in(ctx.communicator, Ice::currentEncoding, p);
            in.read(t);
        }

        static void write(const T& t, MDB_val& val, Ice::OutputStream& holder)
        {
            // Since we use an OutputStream constructed with the default constructor, the encoding is 1.1
            // (aka currentEncoding) and the class format is Compact.
            holder.write(t);
            val.mv_size = holder.b.size();
            val.mv_data = &holder.b[0];
        }

        static bool write(const T& t, MDB_val& val)
        {
            const size_t limit = val.mv_size;
            std::pair<std::byte*, std::byte*> p(
                reinterpret_cast<std::byte*>(val.mv_data),
                reinterpret_cast<std::byte*>(val.mv_data) + limit);
            Ice::OutputStream stream(p);
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

    size_t getMapSize(int);
}

#endif
