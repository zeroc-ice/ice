//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        class Address
        {
            constructor(host:string, port:number);

            host:string;
            port:number;
        }
    }
}
