//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
