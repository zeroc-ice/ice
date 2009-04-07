// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Util
{
    public static Instance
    getInstance(Ice.Communicator communicator)
    {
        Ice.CommunicatorI p = (Ice.CommunicatorI)communicator;
        return p.getInstance();
    }

    public static ProtocolPluginFacade
    getProtocolPluginFacade(Ice.Communicator communicator)
    {
        return new ProtocolPluginFacadeI(communicator);
    }

    //
    // Given a path name, first try to open it as a class path resource (the path is
    // treated as absolute). If that fails, fall back to the file system. Returns null
    // if the file does not exist and raises IOException if an error occurs.
    //
    public static java.io.InputStream
    openResource(ClassLoader cl, String path)
        throws java.io.IOException
    {
        //
        // Calling getResourceAsStream on the class loader means all paths are absolute,
        // whereas calling it on the class means all paths are relative to the class
        // unless the path has a leading forward slash. We call it on the class loader.
        //
        // getResourceAsStream returns null if the resource can't be found.
        //
        java.io.InputStream stream = cl.getResourceAsStream(path);
        if(stream == null)
        {
            try
            {
                java.io.File f = new java.io.File(path);
                if(f.exists())
                {
                    stream = new java.io.FileInputStream(f);
                }
            }
            catch(java.lang.SecurityException ex)
            {
                // Ignore - a security manager may forbid access to the local file system.
            }
        }

        return stream;
    }
}
