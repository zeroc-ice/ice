// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class MyObjectI extends Test._MyObjectDisp
{

    public int 
    add(int x, int y, Ice.Current current)
    {
        return x + y;
    } 
    
    public int 
    addWithRetry(int x, int y, Ice.Current current)
    {
        String val = (String)current.ctx.get("retry");
        
        if(val == null || !val.equals("no"))
        {
            throw new Test.RetryException();
        }
        return x + y;
    } 

    public int 
    badAdd(int x, int y, Ice.Current current) throws Test.InvalidInputException
    {
        throw new Test.InvalidInputException();
    } 

    public int 
    notExistAdd(int x, int y, Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    } 
    
    public int 
    badSystemAdd(int x, int y, Ice.Current current)
    {
        throw new Ice.InitializationException("testing");
    } 


    //
    // AMD
    //

    public void 
    amdAdd_async(final Test.AMD_MyObject_amdAdd cb, final int x, final int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
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

    public void 
    amdAddWithRetry_async(final Test.AMD_MyObject_amdAddWithRetry cb, final int x, final int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
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
        
        String val = (String)current.ctx.get("retry");
        
        if(val == null || !val.equals("no"))
        {
            throw new Test.RetryException();
        }
    } 
    
    public void 
    amdBadAdd_async(final Test.AMD_MyObject_amdBadAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
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
                    cb.ice_exception(new Test.InvalidInputException());
                }
            };
        
        thread.setDaemon(true);
        thread.start();
    } 

    public void 
    amdNotExistAdd_async(final Test.AMD_MyObject_amdNotExistAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
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
    
    public void 
    amdBadSystemAdd_async(final Test.AMD_MyObject_amdBadSystemAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread()
            {
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
                    cb.ice_exception(new Ice.InitializationException("testing"));
                }
            };
        
        thread.setDaemon(true);
        thread.start();
    } 
}