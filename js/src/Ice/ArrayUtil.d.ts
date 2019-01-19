//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        class ArrayUtil
        {
            static clone<T>(arr:T[]):T[];
            static equals(lhs:any[]|Uint8Array, rhs:any[]|Uint8Array, valuesEqual?:(v1:any, v2:any)=>boolean):boolean;
            static shuffle(arr:any[]):void;
        }
    }
}
