// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

declare module "ice"
{
    namespace Ice
    {
        class Long
        {
            constructor(high?:number, low?:number);
            hashCode():number;
            equals(rhs:Long):boolean;
            toString():string;
            toNumber():number;

            low:number;
            high:number;
        }
    }
}
