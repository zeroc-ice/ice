//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
