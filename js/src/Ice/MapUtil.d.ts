//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
