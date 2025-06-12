// Copyright (c) ZeroC, Inc.

#ifndef ICEPHP_UTIL_H
#define ICEPHP_UTIL_H

#include "Config.h"

extern "C"
{
    ZEND_FUNCTION(Ice_stringVersion);
    ZEND_FUNCTION(Ice_intVersion);
    ZEND_FUNCTION(Ice_generateUUID);
    ZEND_FUNCTION(Ice_currentProtocol);
    ZEND_FUNCTION(Ice_currentProtocolEncoding);
    ZEND_FUNCTION(Ice_currentEncoding);
    ZEND_FUNCTION(Ice_protocolVersionToString);
    ZEND_FUNCTION(Ice_stringToProtocolVersion);
    ZEND_FUNCTION(Ice_encodingVersionToString);
    ZEND_FUNCTION(Ice_stringToEncodingVersion);
}

namespace IcePHP
{
    void* createWrapper(zend_class_entry*, size_t);
    void* extractWrapper(zval*);

    // Wraps a C++ pointer inside a PHP object.
    template<typename T> struct Wrapper
    {
        T* ptr;

        static Wrapper<T>* create(zend_class_entry* ce)
        {
            Wrapper<T>* w = static_cast<Wrapper<T>*>(ecalloc(1, sizeof(Wrapper<T>) + zend_object_properties_size(ce)));

            zend_object_std_init(&w->zobj, ce);
            object_properties_init(&w->zobj, ce);

            w->ptr = 0;
            return w;
        }

        static Wrapper<T>* extract(zval* zv)
        {
            return reinterpret_cast<Wrapper<T>*>(
                reinterpret_cast<char*>(extractWrapper(zv)) - XtOffsetOf(Wrapper<T>, zobj));
        }

        static Wrapper<T>* fetch(zend_object* object)
        {
            return reinterpret_cast<Wrapper<T>*>(reinterpret_cast<char*>(object) - XtOffsetOf(Wrapper<T>, zobj));
        }

        static T value(zval* zv)
        {
            Wrapper<T>* w = extract(zv);
            if (w)
            {
                return *w->ptr;
            }
            return 0;
        }

        // This must be last element in the struct
        zend_object zobj;
    };

    zend_class_entry* nameToClass(const std::string&);

    bool createIdentity(zval*, const Ice::Identity&);
    bool extractIdentity(zval*, Ice::Identity&);

    bool createStringMap(zval*, const std::map<std::string, std::string, std::less<>>&);

    inline bool createContext(zval* zv, const Ice::Context& ctx) { return createStringMap(zv, ctx); }
    bool extractContext(zval*, Ice::Context&);

    bool createStringArray(zval*, const Ice::StringSeq&);
    bool extractStringArray(zval*, Ice::StringSeq&);

    // Create a PHP instance of Ice_ProtocolVersion.
    bool createProtocolVersion(zval*, const Ice::ProtocolVersion&);

    // Create a PHP instance of Ice_EncodingVersion.
    bool createEncodingVersion(zval*, const Ice::EncodingVersion&);

    // Extracts the members of an encoding version.
    bool extractEncodingVersion(zval*, Ice::EncodingVersion&);

    // Converts a C++ Ice::LocalException or std::exception (or other C++ exception) to a PHP exception.
    // Other Ice C++ exceptions such as user exceptions are handled as std::exception.
    void convertException(zval*, std::exception_ptr);

    // Converts a C++ exception to a PHP exception and throws it.
    // Uses convertException to convert the exception.
    void throwException(std::exception_ptr);

    // Convert a Zend type (e.g., IS_BOOL, etc.) to a string for use in error messages.
    std::string zendTypeToString(int);

    // Raise RuntimeException with the given message.
    void runtimeError(const std::string&);

    // Raise InvalidArgumentException with the given message.
    void invalidArgument(const std::string&);

    // Invoke a method on a PHP object. The method must not take any arguments.
    bool invokeMethod(zval*, const std::string&);

    // Invoke a method on a PHP object. The method must take one string argument.
    bool invokeMethod(zval*, const std::string&, const std::string&);

    // Check inheritance.
    bool checkClass(zend_class_entry*, zend_class_entry*);

    // Exception-safe efree.
    class AutoEfree
    {
    public:
        AutoEfree(void* p) : _p(p) {}
        ~AutoEfree() { efree(_p); }

    private:
        void* _p;
    };

    // Exception-safe zval destroy.
    class AutoDestroy
    {
    public:
        AutoDestroy(zval* zv) : _zv(zv) {}
        ~AutoDestroy()
        {
            if (_zv)
                zval_ptr_dtor(_zv);
        }

        zval* release()
        {
            zval* z = _zv;
            _zv = 0;
            return z;
        }

    private:
        zval* _zv;
    };

    class AutoReleaseString
    {
    public:
        AutoReleaseString(zend_string* s) : _s(s) {}
        ~AutoReleaseString()
        {
            if (_s)
                zend_string_release(_s);
        }

    private:
        zend_string* _s;
    };

} // End of namespace IcePHP

#endif
