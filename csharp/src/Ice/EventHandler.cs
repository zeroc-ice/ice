//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{

    public abstract class EventHandler
    {
        //
        // Called to start a new asynchronous read or write operation.
        //
        public abstract bool startAsync(int op, AsyncCallback cb, ref bool completedSynchronously);

        public abstract bool finishAsync(int op);

        //
        // Called when there's a message ready to be processed.
        //
        public abstract void message(ref ThreadPoolCurrent op);

        //
        // Called when the event handler is unregistered.
        //
        public abstract void finished(ref ThreadPoolCurrent op);

        internal int _ready = 0;
        internal int _pending = 0;
        internal int _started = 0;
        internal bool _finish = false;

        internal bool _hasMoreData = false;
        internal int _registered = 0;
    }

}
