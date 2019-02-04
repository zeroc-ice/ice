//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        class AsyncResultBase<T> extends Promise<T>
        {
            constructor(communicator:Communicator, op:string, connection:Connection, proxy:ObjectPrx, adapter:ObjectAdapter);
            readonly communicator:Communicator;
            readonly connection:Connection;
            readonly proxy:ObjectPrx;
            readonly adapter:ObjectAdapter;
            readonly operation:string;
        }
    }
}
