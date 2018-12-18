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
