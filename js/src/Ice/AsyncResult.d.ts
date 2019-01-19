//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
