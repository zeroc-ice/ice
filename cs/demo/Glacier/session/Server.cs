// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Server
{
    internal class PermissionsVerifierI : Glacier._PermissionsVerifierDisp
    {
        internal PermissionsVerifierI(Ice.ObjectAdapter adapter)
        {
            _adapter = adapter;
        }
        
        public override bool checkPermissions(string userId, string password, out string reason, Ice.Current current)
        {
            if(!userId.Equals(password))
            {
                reason = "user ID does not match password";
                return false;
            }
            reason = "";
            return true;
        }
        
        private Ice.ObjectAdapter _adapter;
    }
    
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("HelloSessionManager");
        
        //
        // Create the SessionManager.
        //
        Ice.Object @object = new HelloSessionManagerI(adapter);
        adapter.add(@object, Ice.Util.stringToIdentity("HelloSessionManager"));
        
        //
        // Create the PermissionsVerifier.
        //
        @object = new PermissionsVerifierI(adapter);
        adapter.add(@object, Ice.Util.stringToIdentity("PermissionsVerifier"));
        
        adapter.activate();
        communicator.waitForShutdown();
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(ref args, properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }
        
	if(communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(System.Exception ex)
	    {
		Console.Error.WriteLine(ex);
		status = 1;
	    }
	}
        
        Environment.Exit(status);
    }
}
