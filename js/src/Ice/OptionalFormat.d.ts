//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        class OptionalFormat
        {
            static readonly F1:OptionalFormat;
            static readonly F2:OptionalFormat;
            static readonly F4:OptionalFormat;
            static readonly F8:OptionalFormat;
            static readonly Size:OptionalFormat;
            static readonly VSize:OptionalFormat;
            static readonly FSize:OptionalFormat;
            static readonly Class:OptionalFormat;

            static valueOf(value:number):OptionalFormat;
            equals(other:any):boolean;
            hashCode():number;
            toString():string;

            readonly name:string;
            readonly value:number;
        }
    }
}
