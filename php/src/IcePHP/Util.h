// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPHP_UTIL_H
#define ICEPHP_UTIL_H

#include <Config.h>

//
// Global functions.
//
extern "C"
{
ZEND_FUNCTION(Ice_stringVersion);
ZEND_FUNCTION(Ice_intVersion);
ZEND_FUNCTION(Ice_generateUUID);
}

#define ICEPHP_UTIL_FUNCTIONS \
    ZEND_FE(Ice_stringVersion, NULL) \
    ZEND_FE(Ice_intVersion, NULL) \
    ZEND_FE(Ice_generateUUID, NULL)

#ifdef ICEPHP_USE_NAMESPACES
#   define ICEPHP_UTIL_NS_FUNCTIONS \
    ZEND_NS_FALIAS("Ice", stringVersion, Ice_stringVersion, NULL) \
    ZEND_NS_FALIAS("Ice", intVersion, Ice_intVersion, NULL) \
    ZEND_NS_FALIAS("Ice", generateUUID, Ice_generateUUID, NULL)
#else
#   define ICEPHP_UTIL_NS_FUNCTIONS
#endif

namespace IcePHP
{

void* createWrapper(zend_class_entry*, size_t TSRMLS_DC);
void* extractWrapper(zval* TSRMLS_DC);

//
// Wraps a C++ pointer inside a PHP object.
//
template<typename T>
struct Wrapper
{
    zend_object zobj;
    T* ptr;

    static Wrapper<T>* create(zend_class_entry* ce TSRMLS_DC)
    {
        Wrapper<T>* w = static_cast<Wrapper<T>*>(createWrapper(ce, sizeof(Wrapper<T>) TSRMLS_CC));
        w->ptr = 0;
        return w;
    }

    static Wrapper<T>* extract(zval* zv TSRMLS_DC)
    {
        return static_cast<Wrapper<T>*>(extractWrapper(zv TSRMLS_CC));
    }

    static T value(zval* zv TSRMLS_DC)
    {
        Wrapper<T>* w = extract(zv TSRMLS_CC);
        if(w)
        {
            return *w->ptr;
        }
        return 0;
    }
};

zend_class_entry* idToClass(const std::string& TSRMLS_DC);
zend_class_entry* nameToClass(const std::string& TSRMLS_DC);

bool createIdentity(zval*, const Ice::Identity& TSRMLS_DC);
bool extractIdentity(zval*, Ice::Identity& TSRMLS_DC);

bool createStringMap(zval*, const std::map<std::string, std::string>& TSRMLS_DC);
bool extractStringMap(zval*, std::map<std::string, std::string>& TSRMLS_DC);

bool createStringArray(zval*, const Ice::StringSeq& TSRMLS_DC);
bool extractStringArray(zval*, Ice::StringSeq& TSRMLS_DC);

//
// Convert the given exception into its PHP equivalent.
//
zval* convertException(const Ice::Exception& TSRMLS_DC);

//
// Convert the exception and "throw" it.
//
void throwException(const Ice::Exception& TSRMLS_DC);

//
// Convert a Zend type (e.g., IS_BOOL, etc.) to a string for use in error messages.
//
std::string zendTypeToString(int);

//
// Raise RuntimeException with the given message.
//
void runtimeError(const char* TSRMLS_DC, ...);

//
// Raise InvalidArgumentException with the given message.
//
void invalidArgument(const char* TSRMLS_DC, ...);

//
// Invoke a method on a PHP object. The method must not take any arguments.
//
bool invokeMethod(zval*, const std::string& TSRMLS_DC);

//
// Invoke a method on a PHP object. The method must take one string argument.
//
bool invokeMethod(zval*, const std::string&, const std::string& TSRMLS_DC);

//
// Check inheritance.
//
bool checkClass(zend_class_entry*, zend_class_entry*);

//
// Exception-safe efree.
//
class AutoEfree
{
public:
    AutoEfree(void* p) : _p(p) {}
    ~AutoEfree() { efree(_p); }

private:
    void* _p;
};

//
// Exception-safe zval destroy.
//
class AutoDestroy
{
public:
    AutoDestroy(zval* zv) : _zv(zv) {}
    ~AutoDestroy() { if(_zv) zval_ptr_dtor(&_zv); }

    zval* release() { zval* z = _zv; _zv = 0; return z; }

private:
    zval* _zv;
};

} // End of namespace IcePHP

#endif
