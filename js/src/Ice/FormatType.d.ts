//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        class FormatType
        {
            static readonly DefaultFormat:FormatType;
            static readonly CompactFormat:FormatType;
            static readonly SlicedFormat:FormatType;

            static valueOf(value:number):FormatType;
            equals(other:any):boolean;
            hashCode():number;
            toString():string;

            readonly name:string;
            readonly value:number;
        }
    }
}
