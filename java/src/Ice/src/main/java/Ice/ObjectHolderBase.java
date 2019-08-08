// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Holder base class for Ice objects that are in- or inout-parameters.
 **/
public abstract class ObjectHolderBase<T extends Ice.Object> implements ReadObjectCallback, IceInternal.Patcher
{
    /**
     * Instantiates the class with a <code>null</code> value.
     **/
    public
    ObjectHolderBase()
    {
    }

    /**
     * Instantiates the class with the passed Ice object.
     *
     * @param obj The initial value for this holder.
     **/
    public
    ObjectHolderBase(T obj)
    {
        this.value = obj;
    }

    /**
     * Sets the Ice object of this holder to the passed instance.
     *
     * @param obj The new object for this holder.
     **/
    @Override
    public void
    invoke(Ice.Object obj)
    {
        patch(obj);
    }

    /**
     * The Ice object stored by this holder.
     **/
    public T value;
}
