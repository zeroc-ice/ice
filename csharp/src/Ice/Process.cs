//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    // Default implementation of the Process Admin facet.
    internal sealed class Process : IProcess
    {
        private readonly Communicator _communicator;

        public void Shutdown(Current current) => _communicator.Shutdown();

        public void WriteMessage(string message, int fd, Current current)
        {
            switch (fd)
            {
                case 1:
                    System.Console.Out.WriteLine(message);
                    break;
                case 2:
                    System.Console.Error.WriteLine(message);
                    break;
            }
        }

        internal Process(Communicator communicator) => _communicator = communicator;
    }
}
