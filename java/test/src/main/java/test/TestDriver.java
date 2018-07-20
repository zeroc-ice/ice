// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test;

public class TestDriver
{
    public static void main(String[] args)
    {
        int status = 0;
        try
        {
            String testClass = System.getProperty("test.class");
            Class<?> c = Class.forName(testClass);
            test.TestHelper helper = (test.TestHelper)c.getDeclaredConstructor().newInstance();
            helper.run(args);
            helper.getWriter().flush();
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            status = 1;
        }
        finally
        {
            System.gc();
        }
        System.exit(status);
    }
}
