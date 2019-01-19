//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class ProcessI implements com.zeroc.Ice.Process
{
    public ProcessI(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        _communicator.shutdown();
    }

    @Override
    public void writeMessage(String message, int fd, com.zeroc.Ice.Current current)
    {
        switch(fd)
        {
            case 1:
            {
                System.out.println(message);
                break;
            }
            case 2:
            {
                System.err.println(message);
                break;
            }
        }
    }

    private com.zeroc.Ice.Communicator _communicator;
}
