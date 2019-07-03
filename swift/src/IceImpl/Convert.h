//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "LocalObject.h"

#include <exception>
#include <vector>
#include <map>
#include <string>

@class ICERuntimeException;

namespace IceSSL
{
    class Certificate;
}

NSError* convertException(const std::exception_ptr&);
NSError* convertException(const std::exception&);
std::exception_ptr convertException(ICERuntimeException*);

inline NSString*
toNSString(const std::string& s)
{
    return [[NSString alloc] initWithUTF8String:s.c_str()];
}

inline std::string
fromNSString(NSString* s)
{
    return s == nil ? std::string() : [s UTF8String];
}

inline NSObject<NSCopying>*
toObjC(const std::string& s)
{
    return [[NSString alloc] initWithUTF8String:s.c_str()];
}

inline void
fromObjC(id object, std::string& s)
{
    s = object == [NSNull null] ? ::std::string() : [object UTF8String];
}

NSObject* toObjC(const std::shared_ptr<Ice::Endpoint>& endpoint);
void fromObjC(id object, std::shared_ptr<Ice::Endpoint>& endpoint);

NSObject* toObjC(const std::shared_ptr<IceSSL::Certificate>& cert);

template<typename T> NSMutableArray*
toNSArray(const std::vector<T>& seq)
{
    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:seq.size()];
    for(typename std::vector<T>::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        NSObject* obj = toObjC(*p);
        [array addObject:obj];
    }
    return array;
}

template<typename T> std::vector<T>&
fromNSArray(NSArray* array, std::vector<T>& seq)
{
    if(array != nil)
    {
        seq.reserve([array count]);
        NSEnumerator* enumerator = [array objectEnumerator];
        id obj = nil;
        while((obj = [enumerator nextObject]))
        {
            T v;
            fromObjC(obj, v);
            seq.push_back(v);
        }
    }
    return seq;
}

template<typename T> NSMutableData*
toNSData(const std::vector<T>& seq)
{
    NSMutableData* array = [[NSMutableData alloc] initWithLength:seq.size() * sizeof(T)];
    T* target = (T*)[array bytes];
    for(typename std::vector<T>::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        *target++ = *p;
    }
    return array;
}

template<typename K, typename V> NSMutableDictionary*
toNSDictionary(const std::map<K, V>& dict)
{
    NSMutableDictionary* dictionary = [[NSMutableDictionary alloc] initWithCapacity:dict.size()];
    for(typename std::map<K, V>::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
        NSObject<NSCopying>* key = toObjC(p->first);
        NSObject* value = toObjC(p->second);
        [dictionary setObject:value forKey:key];
    }
    return dictionary;
}

template<typename K, typename V> std::map<K, V>&
fromNSDictionary(NSDictionary* dictionary, std::map<K, V>& dict)
{
    if(dictionary != nil)
    {
        NSEnumerator* enumerator = [dictionary keyEnumerator];
        id obj = nil;
        while((obj = [enumerator nextObject]))
        {
            K k;
            fromObjC(obj, k);
            V v;
            fromObjC([dictionary objectForKey:obj], v);
            dict.insert(std::pair<K, V>(k, v));
        }
    }
    return dict;
}
