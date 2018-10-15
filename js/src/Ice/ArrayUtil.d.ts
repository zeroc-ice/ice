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
        class ArrayUtil
        {
            static clone<T>(arr:T[]):T[];
            static equals(lhs:any[]|Uint8Array, rhs:any[]|Uint8Array, valuesEqual?:(v1:any, v2:any)=>boolean):boolean;
            static shuffle(arr:any[]):void;
        }
    }
}
