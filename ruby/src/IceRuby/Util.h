// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_UTIL_H
#define ICE_RUBY_UTIL_H

#include "Config.h"
#include "Ice/Ice.h"

//
// Avoid clang conversion warnings in "callRuby" calls
//
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wconversion"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#endif

namespace IceRuby
{
    void initUtil(VALUE);

    class RubyException
    {
    public:
        //
        // This constructor uses the interpreter's last error result as the exception.
        //
        RubyException();

        //
        // The Ruby exception object is supplied.
        //
        RubyException(VALUE);

        //
        // The Ruby exception object is supplied along with a message.
        //
        RubyException(VALUE, const char*, ...);

        std::ostream& operator<<(std::ostream&) const;

        VALUE ex;
    };

    //
    // Returns true if the value is a string or can be converted into a string.
    //
    bool isString(VALUE);

    //
    // Returns true if the value is an array or can be converted into an array.
    //
    bool isArray(VALUE);

    //
    // Returns true if the value is a hash or can be converted into a hash.
    //
    bool isHash(VALUE);

    //
    // Convert a Ruby value into a string. May raise RubyException.
    //
    std::string getString(VALUE);

    //
    // Create a Ruby string. May raise RubyException.
    //
    VALUE createString(std::string_view);

    //
    // Convert a Ruby value into a long. May raise RubyException.
    //
    long getInteger(VALUE);

    //
    // Convert a Ruby value into an std::int64_t. May raise RubyException.
    //
    std::int64_t getLong(VALUE);

    //
    // Convert a Ruby array into a vector<string>. Returns true on
    // success and false if the value is not an array. May raise
    // RubyException.
    //
    bool arrayToStringSeq(VALUE, std::vector<std::string>&);

    //
    // Convert a vector of strings into a Ruby array. May raise
    // RubyException.
    //
    VALUE stringSeqToArray(const std::vector<std::string>&);

    //
    // Convert a vector of std::byte into a Ruby array of numbers.
    // May raise RubyException.
    //
    VALUE createNumSeq(const std::vector<std::byte>&);

    //
    // Convert a Ruby hash to Ice::Context. Returns true on success
    // and false if the value is not a hash. May raise RubyException.
    //
    bool hashToContext(VALUE, Ice::Context&);

    //
    // Convert Ice::Context to a hash. May raise RubyException.
    //
    VALUE contextToHash(const Ice::Context&);

    //
    // Abstract class representing an iterator for a Ruby hash collection.
    //
    class HashIterator
    {
    public:
        virtual ~HashIterator() = default;

        virtual void element(VALUE, VALUE) = 0;
    };

    //
    // Iterate over the elements in a Ruby hash. The iterator's
    // element method is invoked for each entry. May raise
    // RubyException.
    //
    void hashIterate(VALUE, HashIterator&);

    //
    // Convert a Ruby value into Ice::Identity. May raise RubyException.
    //
    Ice::Identity getIdentity(VALUE);

    //
    // Create an instance of Ice::Identity. May raise RubyException.
    //
    VALUE createIdentity(const Ice::Identity&);

    //
    // Create a Ruby instance of Ice.ProtocolVersion.
    //
    VALUE createProtocolVersion(const Ice::ProtocolVersion&);

    //
    // Create a Ruby instance of Ice.EncodingVersion.
    //
    VALUE createEncodingVersion(const Ice::EncodingVersion&);

    //
    // Extracts the members of an encoding version.
    //
    bool getEncodingVersion(VALUE, Ice::EncodingVersion&);

    //
    // The callRuby functions are used to invoke Ruby C API functions
    // while translating any Ruby exception into RubyException so that
    // C++ objects are cleaned up properly. Overloadings are provided
    // to support API functions that accept multiple arguments.
    //
    template<typename Fun> VALUE callRuby(Fun fun);

    template<typename Fun, typename T1> VALUE callRuby(Fun fun, T1 t1);

    template<typename Fun, typename T1, typename T2> VALUE callRuby(Fun fun, T1 t1, T2 t2);

    template<typename Fun, typename T1, typename T2, typename T3> VALUE callRuby(Fun fun, T1 t1, T2 t2, T3 t3);

    template<typename Fun, typename T1, typename T2, typename T3, typename T4>
    VALUE callRuby(Fun fun, T1 t1, T2 t2, T3 t3, T4 t4);

    template<typename Fun, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    VALUE callRuby(Fun fun, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6);

    extern "C" typedef VALUE (*RubyFunction)(VALUE);

    VALUE callProtected(RubyFunction, VALUE);

    template<typename Fun> class RF_0
    {
    public:
        RF_0(Fun f) : _f(f) {}
        inline VALUE operator()() { return _f(); }
        static inline VALUE call(VALUE f) { return (*reinterpret_cast<RF_0*>(f))(); }

    private:
        Fun _f;
    };

    template<typename Fun> inline VALUE callRuby(Fun fun)
    {
        typedef RF_0<Fun> RF;
        RF f(fun);
        return callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1> class RF_1
    {
    public:
        RF_1(Fun f, T1 t1) : _f(f), _t1(t1) {}
        inline VALUE operator()() { return _f(_t1); }
        static inline VALUE call(VALUE f) { return (*reinterpret_cast<RF_1*>(f))(); }

    private:
        Fun _f;
        T1 _t1;
    };

    template<typename Fun, typename T1> inline VALUE callRuby(Fun fun, T1 t1)
    {
        typedef RF_1<Fun, T1> RF;
        RF f(fun, t1);
        return callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1, typename T2> class RF_2
    {
    public:
        RF_2(Fun f, T1 t1, T2 t2) : _f(f), _t1(t1), _t2(t2) {}
        inline VALUE operator()() { return _f(_t1, _t2); }
        static inline VALUE call(VALUE f) { return (*reinterpret_cast<RF_2*>(f))(); }

    private:
        Fun _f;
        T1 _t1;
        T2 _t2;
    };

    template<typename Fun, typename T1, typename T2> inline VALUE callRuby(Fun fun, T1 t1, T2 t2)
    {
        typedef RF_2<Fun, T1, T2> RF;
        RF f(fun, t1, t2);
        return callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1, typename T2, typename T3> class RF_3
    {
    public:
        RF_3(Fun f, T1 t1, T2 t2, T3 t3) : _f(f), _t1(t1), _t2(t2), _t3(t3) {}
        inline VALUE operator()() { return _f(_t1, _t2, _t3); }
        static inline VALUE call(VALUE f) { return (*reinterpret_cast<RF_3*>(f))(); }

    private:
        Fun _f;
        T1 _t1;
        T2 _t2;
        T3 _t3;
    };

    template<typename Fun, typename T1, typename T2, typename T3> inline VALUE callRuby(Fun fun, T1 t1, T2 t2, T3 t3)
    {
        typedef RF_3<Fun, T1, T2, T3> RF;
        RF f(fun, t1, t2, t3);
        return callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1, typename T2, typename T3, typename T4> class RF_4
    {
    public:
        RF_4(Fun f, T1 t1, T2 t2, T3 t3, T4 t4) : _f(f), _t1(t1), _t2(t2), _t3(t3), _t4(t4) {}
        inline VALUE operator()() { return _f(_t1, _t2, _t3, _t4); }
        static inline VALUE call(VALUE f) { return (*reinterpret_cast<RF_4*>(f))(); }

    private:
        Fun _f;
        T1 _t1;
        T2 _t2;
        T3 _t3;
        T4 _t4;
    };

    template<typename Fun, typename T1, typename T2, typename T3, typename T4>
    inline VALUE callRuby(Fun fun, T1 t1, T2 t2, T3 t3, T4 t4)
    {
        typedef RF_4<Fun, T1, T2, T3, T4> RF;
        RF f(fun, t1, t2, t3, t4);
        return callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> class RF_6
    {
    public:
        RF_6(Fun f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
            : _f(f),
              _t1(t1),
              _t2(t2),
              _t3(t3),
              _t4(t4),
              _t5(t5),
              _t6(t6)
        {
        }
        inline VALUE operator()() { return _f(_t1, _t2, _t3, _t4, _t5, _t6); }
        static inline VALUE call(VALUE f) { return (*reinterpret_cast<RF_6*>(f))(); }

    private:
        Fun _f;
        T1 _t1;
        T2 _t2;
        T3 _t3;
        T4 _t4;
        T5 _t5;
        T6 _t6;
    };

    template<typename Fun, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    inline VALUE callRuby(Fun fun, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    {
        typedef RF_6<Fun, T1, T2, T3, T4, T5, T6> RF;
        RF f(fun, t1, t2, t3, t4, t5, t6);
        return callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    //
    // The callRubyVoid functions are used to invoke Ruby C API functions
    // while translating any Ruby exception into RubyException so that
    // C++ objects are cleaned up properly. Overloadings are provided
    // to support API functions that accept multiple arguments.
    //
    template<typename Fun> void callRubyVoid(Fun fun);

    template<typename Fun, typename T1> void callRubyVoid(Fun fun, T1 t1);

    template<typename Fun, typename T1, typename T2> void callRubyVoid(Fun fun, T1 t1, T2 t2);

    template<typename Fun, typename T1, typename T2, typename T3> void callRubyVoid(Fun fun, T1 t1, T2 t2, T3 t3);

    template<typename Fun, typename T1, typename T2, typename T3, typename T4>
    void callRubyVoid(Fun fun, T1 t1, T2 t2, T3 t3, T4 t4);

    template<typename Fun> class RFV_0
    {
    public:
        RFV_0(Fun f) : _f(f) {}
        inline void operator()() { _f(); }

        static inline VALUE call(VALUE f)
        {
            (*reinterpret_cast<RFV_0*>(f))();
            return Qnil;
        }

    private:
        Fun _f;
    };

    template<typename Fun> inline void callRubyVoid(Fun fun)
    {
        typedef RFV_0<Fun> RF;
        RF f(fun);
        callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1> class RFV_1
    {
    public:
        RFV_1(Fun f, T1 t1) : _f(f), _t1(t1) {}
        inline void operator()() { _f(_t1); }
        static inline VALUE call(VALUE f)
        {
            (*reinterpret_cast<RFV_1*>(f))();
            return Qnil;
        }

    private:
        Fun _f;
        T1 _t1;
    };

    template<typename Fun, typename T1> inline void callRubyVoid(Fun fun, T1 t1)
    {
        typedef RFV_1<Fun, T1> RF;
        RF f(fun, t1);
        callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1, typename T2> class RFV_2
    {
    public:
        RFV_2(Fun f, T1 t1, T2 t2) : _f(f), _t1(t1), _t2(t2) {}
        inline void operator()() { _f(_t1, _t2); }
        static inline VALUE call(VALUE f)
        {
            (*reinterpret_cast<RFV_2*>(f))();
            return Qnil;
        }

    private:
        Fun _f;
        T1 _t1;
        T2 _t2;
    };

    template<typename Fun, typename T1, typename T2> inline void callRubyVoid(Fun fun, T1 t1, T2 t2)
    {
        typedef RFV_2<Fun, T1, T2> RF;
        RF f(fun, t1, t2);
        callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1, typename T2, typename T3> class RFV_3
    {
    public:
        RFV_3(Fun f, T1 t1, T2 t2, T3 t3) : _f(f), _t1(t1), _t2(t2), _t3(t3) {}
        inline void operator()() { _f(_t1, _t2, _t3); }
        static inline VALUE call(VALUE f)
        {
            (*reinterpret_cast<RFV_3*>(f))();
            return Qnil;
        }

    private:
        Fun _f;
        T1 _t1;
        T2 _t2;
        T3 _t3;
    };

    template<typename Fun, typename T1, typename T2, typename T3> inline void callRubyVoid(Fun fun, T1 t1, T2 t2, T3 t3)
    {
        typedef RFV_3<Fun, T1, T2, T3> RF;
        RF f(fun, t1, t2, t3);
        callProtected(RF::call, reinterpret_cast<VALUE>(&f));
    }

    template<typename Fun, typename T1, typename T2, typename T3, typename T4> class RFV_4
    {
    public:
        RFV_4(Fun f, T1 t1, T2 t2, T3 t3, T4 t4) : _f(f), _t1(t1), _t2(t2), _t3(t3), _t4(t4) {}
        inline void operator()() { _f(_t1, _t2, _t3, _t4); }
        static inline VALUE call(VALUE f)
        {
            (*reinterpret_cast<RFV_4*>(f))();
            return Qnil;
        }

    private:
        Fun _f;
        T1 _t1;
        T2 _t2;
        T3 _t3;
        T4 _t4;
    };

    template<typename Fun, typename T1, typename T2, typename T3, typename T4>
    inline void callRubyVoid(Fun fun, T1 t1, T2 t2, T3 t3, T4 t4)
    {
        typedef RFV_4<Fun, T1, T2, T3, T4> RF;
        RF f(fun, t1, t2, t3, t4);
        callProtected(RubyFunction(RF::call), reinterpret_cast<VALUE>(&f));
    }

    //
    // Create an array with the given size. May raise RubyException.
    //
    // Note that the length of the array returned by this function is already
    // set to the requested size. This prevents the array's elements from being
    // prematurely garbage-collected, but it means the array must be populated
    // via direct access to its buffer and not by pushing elements onto the
    // array using rb_ary_push:
    //
    // VALUE arr = createArray(size);
    // for(long i = 0; i < size; ++i)
    // {
    //     RARRAY_ASET(arr, i, val);
    // }
    //
    VALUE createArray(long);

    // Convert an Ice::LocalException, std::exception or other C++ exception into an Ice Ruby local exception.
    // Ice C++ exceptions that are not LocalException are handled as std::exceptions.
    VALUE convertException(std::exception_ptr);
}

//
// The macros ICE_RUBY_TRY and ICE_RUBY_CATCH must be used in place of try/catch in
// every entry point into the extension. They handle the translation of C++
// exceptions into Ruby exceptions and ensure that C++ objects are cleaned up properly.
//
#define ICE_RUBY_TRY                                                                                                   \
    volatile VALUE ex_ = Qnil;                                                                                         \
    try

#define ICE_RUBY_RETHROW(ex)                                                                                           \
    ex_ = ex;                                                                                                          \
    rb_exc_raise(ex_);

#define ICE_RUBY_CATCH                                                                                                 \
    catch (const ::IceRuby::RubyException& ex) { ICE_RUBY_RETHROW(ex.ex); }                                            \
    catch (const std::bad_alloc& ex) { ICE_RUBY_RETHROW(rb_exc_new2(rb_eNoMemError, ex.what())); }                     \
    catch (...) { ICE_RUBY_RETHROW(convertException(current_exception())); }

#endif
