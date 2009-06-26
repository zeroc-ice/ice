// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

//
// We need to override the resolveClass method of ObjectInputStream so
// that we can use the same class-lookup mechanism as elsewhere in the
// Ice run time.
//

public class ObjectInputStream extends java.io.ObjectInputStream
{
    public
    ObjectInputStream(java.io.InputStream in)
        throws java.io.IOException
    {
        super(in);
    }

    protected Class<?>
    resolveClass(java.io.ObjectStreamClass cls)
        throws java.io.IOException, ClassNotFoundException
    {
        try
        {
            Class<?> c = Util.findClass(cls.getName());
            if(c != null)
            {
                return c;
            }
            throw new ClassNotFoundException("unable to resolve class" + cls.getName());
        }
        catch(Exception ex)
        {
            throw new ClassNotFoundException("unable to resolve class " + cls.getName(), ex);
        }
    }
}
