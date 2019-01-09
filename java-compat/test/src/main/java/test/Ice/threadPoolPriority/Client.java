// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.threadPoolPriority;

import test.Ice.threadPoolPriority.Test.PriorityPrx;
import test.Ice.threadPoolPriority.Test.PriorityPrxHelper;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        try(Ice.Communicator communicator = initialize(args))
        {
            java.io.PrintWriter out = getWriter();
            Ice.ObjectPrx object = communicator().stringToProxy("test:" + getTestEndpoint(0) + " -t 10000");
            PriorityPrx priority = PriorityPrxHelper.checkedCast(object);
            out.print("testing thread priority... ");
            out.flush();
            int prio = priority.getPriority();
            test(prio == 10);
            out.println("ok");

            priority.shutdown();
        }
    }
}
