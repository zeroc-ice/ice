// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class Current
{
    public String identity;

    public String facet;

    public String operation;

    public boolean nonmutating;

    public java.util.HashMap context;

    public ObjectPrx proxy;

    public final void
    __write(IceInternal.BasicStream __os)
    {
        __os.writeString(identity);
        __os.writeString(facet);
        __os.writeString(operation);
        __os.writeBool(nonmutating);
        if (context == null)
        {
            __os.writeInt(0);
        }
        else
        {
            final int sz = context.size();
            __os.writeInt(sz);
            if (sz > 0)
            {
                java.util.Iterator i = context.entrySet().iterator();
                while (i.hasNext())
                {
                    java.util.Map.Entry entry = (java.util.Map.Entry)i.next();
                    __os.writeString((String)entry.getKey());
                    __os.writeString((String)entry.getValue());
                }
            }
        }
        __os.writeProxy(proxy);
    }

    public final void
    __read(IceInternal.BasicStream __is)
    {
        identity = __is.readString();
        facet = __is.readString();
        operation = __is.readString();
        nonmutating = __is.readBool();
        int sz = __is.readInt();
        if (sz > 0)
        {
            context = new java.util.HashMap();
            for (int i = 0; i < sz; i++)
            {
                String key = __is.readString();
                String value = __is.readString();
                context.put(key, value);
            }
        }
        proxy = __is.readProxy();
    }
}
