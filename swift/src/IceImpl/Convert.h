// Copyright (c) ZeroC, Inc.
#import "LocalObject.h"

#include <exception>
#include <map>
#include <string>
#include <vector>

NSError* convertException(std::exception_ptr);

inline NSString*
toNSString(const std::string& s)
{
    return [[NSString alloc] initWithUTF8String:s.c_str()];
}

inline NSString*
toNSString(const char* s)
{
    return [[NSString alloc] initWithUTF8String:s];
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
    s = object == [NSNull null] ? std::string() : [object UTF8String];
}

NSObject* toObjC(const std::shared_ptr<Ice::Endpoint>& endpoint);
void fromObjC(id object, std::shared_ptr<Ice::Endpoint>& endpoint);

template<typename T>
NSMutableArray*
toNSArray(const std::vector<T>& seq)
{
    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:seq.size()];
    for (typename std::vector<T>::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        NSObject* obj = toObjC(*p);
        [array addObject:obj];
    }
    return array;
}

template<typename T>
std::vector<T>&
fromNSArray(NSArray* array, std::vector<T>& seq)
{
    if (array != nil)
    {
        seq.reserve([array count]);
        NSEnumerator* enumerator = [array objectEnumerator];
        id obj = nil;
        while ((obj = [enumerator nextObject]))
        {
            T v;
            fromObjC(obj, v);
            seq.push_back(v);
        }
    }
    return seq;
}

template<typename T>
NSMutableData*
toNSData(const std::vector<T>& seq)
{
    NSMutableData* array = [[NSMutableData alloc] initWithLength:seq.size() * sizeof(T)];
    T* target = (T*)[array bytes];
    for (typename std::vector<T>::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        *target++ = *p;
    }
    return array;
}

template<typename K, typename V, class Compare = std::less<K>>
NSMutableDictionary*
toNSDictionary(const std::map<K, V, Compare>& dict)
{
    NSMutableDictionary* dictionary = [[NSMutableDictionary alloc] initWithCapacity:dict.size()];
    for (const auto& p : dict)
    {
        NSObject<NSCopying>* key = toObjC(p.first);
        NSObject* value = toObjC(p.second);
        [dictionary setObject:value forKey:key];
    }
    return dictionary;
}

template<typename K, typename V, class Compare = std::less<K>>
std::map<K, V, Compare>&
fromNSDictionary(NSDictionary* dictionary, std::map<K, V, Compare>& dict)
{
    if (dictionary != nil)
    {
        NSEnumerator* enumerator = [dictionary keyEnumerator];
        id obj = nil;
        while ((obj = [enumerator nextObject]))
        {
            K k;
            fromObjC(obj, k);
            V v;
            fromObjC([dictionary objectForKey:obj], v);
            dict.emplace(std::move(k), std::move(v));
        }
    }
    return dict;
}
