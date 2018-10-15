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
        function stringToIdentity(s:string):Identity;
        function identityToString(ident:Identity, toStringMode?:ToStringMode):string;
        function proxyIdentityCompare(lhs:ObjectPrx, rhs:ObjectPrx):number;
        function proxyIdentityAndFacetCompare(lhs:ObjectPrx, rhs:ObjectPrx):number;
    }
}
