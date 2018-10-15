// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

declare module "ice"
{
    namespace Ice
    {
        class MapUtil
        {
            static equals<K, V>(lhs:Map<K,V>, rhs:Map<K, V>):boolean;
        }
    }
}
