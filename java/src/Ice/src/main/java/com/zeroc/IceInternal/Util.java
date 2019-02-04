//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import java.util.concurrent.ThreadFactory;

public final class Util
{
    static String
    createThreadName(final com.zeroc.Ice.Properties properties, final String name)
    {
        String threadName = properties.getProperty("Ice.ProgramName");
        if(threadName.length() > 0)
        {
            threadName += "-";
        }

        threadName = threadName + name;
        return threadName;
    }

    static ThreadFactory
    createThreadFactory(final com.zeroc.Ice.Properties properties, final String name)
    {
        return new java.util.concurrent.ThreadFactory()
        {
            @Override
            public Thread newThread(Runnable r)
            {
                Thread t = new Thread(r);
                t.setName(name);

                if(properties.getProperty("Ice.ThreadPriority").length() > 0)
                {
                    t.setPriority(Util.getThreadPriorityProperty(properties, "Ice"));
                }
                return t;
            }
        };
    }

    public static Instance
    getInstance(com.zeroc.Ice.Communicator communicator)
    {
        com.zeroc.Ice.CommunicatorI p = (com.zeroc.Ice.CommunicatorI)communicator;
        return p.getInstance();
    }

    public static ProtocolPluginFacade
    getProtocolPluginFacade(com.zeroc.Ice.Communicator communicator)
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
        java.io.InputStream stream = null;
        try
        {
            stream = cl.getResourceAsStream(path);
        }
        catch(IllegalArgumentException ex)
        {
            //
            // With JDK-7 this can happen if the result url (base url + path) produces a
            // malformed url for an URLClassLoader. For example the code in following
            // comment will produce this exception under Windows.
            //
            // URLClassLoader cl = new URLClassLoader(new URL[] {new URL("http://localhost:8080/")});
            // java.io.InputStream in = Util.openResource(cl, "c:\\foo.txt");
            //
        }
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

    public static Class<?>
    findClass(String className, ClassLoader cl)
        throws LinkageError
    {
        //
        // Try to load the class using the given class loader (if any). If that fails (or
        // none is provided), we try to load the class a few more ways before giving up.
        //
        // Calling Class.forName() doesn't always work. For example, if Ice.jar is installed
        // as an extension (in $JAVA_HOME/jre/lib/ext), calling Class.forName(name) uses the
        // extension class loader, which will not look in CLASSPATH for the target class.
        //

        Class<?> c = null;

        if(cl != null)
        {
            c = loadClass(className, cl);
        }

        //
        // Try using the current thread's class loader.
        //
        if(c == null)
        {
            try
            {
                cl = Thread.currentThread().getContextClassLoader();
                if(cl != null)
                {
                    c = loadClass(className, cl);
                }
            }
            catch(SecurityException ex)
            {
            }
        }

        //
        // Try using Class.forName().
        //
        try
        {
            if(c == null)
            {
                c = Class.forName(className);
            }
        }
        catch(ClassNotFoundException ex)
        {
            // Ignore
        }

        //
        // Fall back to the system class loader (which knows about CLASSPATH).
        //
        if(c == null)
        {
            try
            {
                cl = ClassLoader.getSystemClassLoader();
                if(cl != null)
                {
                    c = loadClass(className, cl);
                }
            }
            catch(SecurityException ex)
            {
            }
        }

        return c;
    }

    private static Class<?>
    loadClass(String className, ClassLoader cl)
    {
        if(cl != null)
        {
            try
            {
                return cl.loadClass(className);
            }
            catch(ClassNotFoundException ex)
            {
                // Ignore
            }
        }

        return null;
    }

    public static int
    getThreadPriorityProperty(com.zeroc.Ice.Properties properties, String prefix)
    {
        String pri = properties.getProperty(prefix + ".ThreadPriority");
        if(pri.equals("MIN_PRIORITY") || pri.equals("java.lang.Thread.MIN_PRIORITY"))
        {
            return java.lang.Thread.MIN_PRIORITY;
        }
        else if(pri.equals("NORM_PRIORITY") || pri.equals("java.lang.Thread.NORM_PRIORITY"))
        {
            return java.lang.Thread.NORM_PRIORITY;
        }
        else if(pri.equals("MAX_PRIORITY") || pri.equals("java.lang.Thread.MAX_PRIORITY"))
        {
            return java.lang.Thread.MAX_PRIORITY;
        }

        try
        {
            return Integer.parseInt(pri);
        }
        catch(NumberFormatException ex)
        {
        }
        return java.lang.Thread.NORM_PRIORITY;
    }

    private static String fixKwd(String name)
    {
        //
        // Keyword list. *Must* be kept in alphabetical order. Note that checkedCast and uncheckedCast
        // are not Java keywords, but are in this list to prevent illegal code being generated if
        // someone defines Slice operations with that name.
        //
        final String[] keywordList =
        {
            "abstract", "assert", "boolean", "break", "byte", "case", "catch",
            "char", "checkedCast", "class", "clone", "const", "continue", "default", "do",
            "double", "else", "enum", "equals", "extends", "false", "final", "finalize",
            "finally", "float", "for", "getClass", "goto", "hashCode", "if",
            "implements", "import", "instanceof", "int", "interface", "long",
            "native", "new", "notify", "notifyAll", "null", "package", "private",
            "protected", "public", "return", "short", "static", "strictfp", "super", "switch",
            "synchronized", "this", "throw", "throws", "toString", "transient",
            "true", "try", "uncheckedCast", "void", "volatile", "wait", "while"
        };
        boolean found =  java.util.Arrays.binarySearch(keywordList, name) >= 0;
        return found ? "_" + name : name;
    }

    //
    // Return true if we're running on Android.
    //
    public static boolean isAndroid()
    {
        return System.getProperty("java.vm.name").startsWith("Dalvik");
    }
}
