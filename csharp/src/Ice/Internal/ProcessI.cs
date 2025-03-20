// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class ProcessI : Ice.ProcessDisp_
{
    public ProcessI(Ice.Communicator communicator) => _communicator = communicator;

    public override void shutdown(Ice.Current current) => _communicator.shutdown();

    public override void writeMessage(string message, int fd, Ice.Current current)
    {
        switch (fd)
        {
            case 1:
            {
                Console.Out.WriteLine(message);
                break;
            }
            case 2:
            {
                Console.Error.WriteLine(message);
                break;
            }
        }
    }

    private readonly Ice.Communicator _communicator;
}
