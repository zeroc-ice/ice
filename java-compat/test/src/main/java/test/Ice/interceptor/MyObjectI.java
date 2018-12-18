// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.interceptor;

import test.Ice.interceptor.Test.AMD_MyObject_amdAdd;
import test.Ice.interceptor.Test.AMD_MyObject_amdAddWithRetry;
import test.Ice.interceptor.Test.AMD_MyObject_amdBadAdd;
import test.Ice.interceptor.Test.AMD_MyObject_amdBadSystemAdd;
import test.Ice.interceptor.Test.AMD_MyObject_amdNotExistAdd;
import test.Ice.interceptor.Test.InvalidInputException;
import test.Ice.interceptor.Test.RetryException;
import test.Ice.interceptor.Test._MyObjectDisp;

class MyObjectI extends _MyObjectDisp
{

    @Override
    public int
    add(int x, int y, Ice.Current current)
    {
        return x + y;
    }

    @Override
    public int
    addWithRetry(int x, int y, Ice.Current current)
    {
        String val = current.ctx.get("retry");

        if(val == null || !val.equals("no"))
        {
            throw new RetryException();
        }
        return x + y;
    }

    @Override
    public int
    badAdd(int x, int y, Ice.Current current) throws InvalidInputException
    {
        throw new InvalidInputException();
    }

    @Override
    public int
    notExistAdd(int x, int y, Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    @Override
    public int
    badSystemAdd(int x, int y, Ice.Current current)
    {
        throw new MySystemException();
    }

    //
    // AMD
    //

    @Override
    public void
    amdAdd_async(final AMD_MyObject_amdAdd cb, final int x, final int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
                @Override
                public void
                run()
                {
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(InterruptedException e)
                    {
                    }
                    cb.ice_response(x + y);
                }
            };

        thread.setDaemon(true);
        thread.start();
    }

    @Override
    public void
    amdAddWithRetry_async(final AMD_MyObject_amdAddWithRetry cb, final int x, final int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
                @Override
                public void
                run()
                {
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(InterruptedException e)
                    {
                    }
                    cb.ice_response(x + y);
                }
            };

        thread.setDaemon(true);
        thread.start();

        String val = current.ctx.get("retry");

        if(val == null || !val.equals("no"))
        {
            cb.ice_exception(new RetryException());
        }
    }

    @Override
    public void
    amdBadAdd_async(final AMD_MyObject_amdBadAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
                @Override
                public void
                run()
                {
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(InterruptedException e)
                    {
                    }
                    cb.ice_exception(new InvalidInputException());
                }
            };

        thread.setDaemon(true);
        thread.start();
    }

    @Override
    public void
    amdNotExistAdd_async(final AMD_MyObject_amdNotExistAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
                @Override
                public void
                run()
                {
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(InterruptedException e)
                    {
                    }
                    cb.ice_exception(new Ice.ObjectNotExistException());
                }
            };

        thread.setDaemon(true);
        thread.start();
    }

    @Override
    public void
    amdBadSystemAdd_async(final AMD_MyObject_amdBadSystemAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
                @Override
                public void
                run()
                {
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(InterruptedException e)
                    {
                    }
                    cb.ice_exception(new MySystemException());
                }
            };

        thread.setDaemon(true);
        thread.start();
    }
}
