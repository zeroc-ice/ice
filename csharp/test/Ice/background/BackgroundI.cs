//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Background
{
    internal class Background : IBackground
    {
        public void op(Current current)
        {
            _controller.checkCallPause(current);
        }

        public void opWithPayload(byte[] seq, Current current)
        {
            _controller.checkCallPause(current);
        }

        public void shutdown(Current current)
        {
            current.Adapter.Communicator.Shutdown();
        }

        internal Background(BackgroundController controller)
        {
            _controller = controller;
        }

        private BackgroundController _controller;
    }
}
