// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public class ProcessI extends Ice._ProcessDisp
{
    public ProcessI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    @Override
    public void
    writeMessage(String message, int fd, Ice.Current current)
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

    private Ice.Communicator _communicator;
}
