// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public class Client
{
    internal class TestClient : Ice.Application
    {
        public override int run(string[] args)
        {
            TestPrx obj = AllTests.allTests(communicator());
            
            System.Console.Out.Write("testing whether server is gone... ");
            System.Console.Out.Flush();
            try
            {
                obj.ice_ping();
                throw new System.Exception();
            }
            catch(System.Exception)
            {
                System.Console.Out.WriteLine("ok");
            }
            
            return 0;
        }
    }
    
    public static void Main(string[] args)
    {
	System.Console.Write(Ice.Util.generateUUID().ToUpper());
	System.Environment.Exit(0);
        TestClient app = new TestClient();
        int result = app.main(args);
        System.Environment.Exit(result);
    }
}
