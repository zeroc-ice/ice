//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceUtilInternal;

public final class Assert
{
    //
    // The JVM ignores exceptions raised in finalizers, therefore finalizers
    // that use assertions should call this method instead of assert().
    //
    public static void
    FinalizerAssert(boolean b)
    {
        if(!b)
        {
            //
            // Create a Throwable to obtain the stack trace.
            //
            Throwable t = new Throwable();
            StackTraceElement[] trace = t.getStackTrace();
            if(trace.length > 1)
            {
                //
                // Skip the first frame, which represents this method.
                //
                System.err.println("Assertion failure:");
                for(StackTraceElement e : trace)
                {
                    System.err.println("\tat " + e);
                }
            }
            else
            {
                System.err.println("Assertion failure (no stack trace information)");
            }
        }
    }
}
