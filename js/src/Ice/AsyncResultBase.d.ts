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
