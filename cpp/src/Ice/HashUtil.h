// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_HASH_UTIL_H
#define ICE_HASH_UTIL_H

namespace IceInternal
{

inline void
hashAdd(Ice::Int& hashCode, Ice::Int value)
{
    hashCode = ((hashCode << 5) + hashCode) ^ (2654435761u * value);
}

inline void
hashAdd(Ice::Int& hashCode, bool value)
{
    hashCode = ((hashCode << 5) + hashCode) ^ (value ? 1 : 0);
}

inline void
hashAdd(Ice::Int& hashCode, const std::string& value)
{
    for(std::string::const_iterator p = value.begin(); p != value.end(); ++p)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ *p;
    }    
}

template<typename T> void
hashAdd(Ice::Int& hashCode, const std::vector<T>& seq)
{
    for(typename std::vector<T>::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        hashAdd(hashCode, *p);
    }
}

template<typename K, typename V> void
hashAdd(Ice::Int& hashCode, const std::map<K, V>& map)
{
    for(typename std::map<K, V>::const_iterator p = map.begin(); p != map.end(); ++p)
    {
        hashAdd(hashCode, p->first);
        hashAdd(hashCode, p->second);
    }
}

}

#endif

