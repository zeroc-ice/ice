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
        class AsyncResult<T> extends AsyncResultBase<T>
        {
            constructor(communicator:Communicator, operation:string, connection:Connection, proxy: ObjectPrx, adapter:ObjectAdapter,
                        completed:(result:AsyncResult<T>) => void);
            cancel():void;
            isCompleted():boolean;
            isSent():boolean;
            throwLocalException():void;
            sentSynchronously():boolean;
        }
    }
}
