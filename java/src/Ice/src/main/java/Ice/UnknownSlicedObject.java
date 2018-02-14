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
 * Unknown sliced object holds an instance of unknown type.
 **/
public final class UnknownSlicedObject extends ObjectImpl
{
    /**
     * Instantiates the class for an Ice object having the given Slice type.
     *
     * @param unknownTypeId The Slice type ID of the unknown object.
     **/
    public
    UnknownSlicedObject(String unknownTypeId)
    {
        _unknownTypeId = unknownTypeId;
    }

    /**
     * Determine the Slice type ID associated with this object.
     *
     * @return The type ID.
     **/
    public String
    getUnknownTypeId()
    {
        return _unknownTypeId;
    }

    @Override
    public void
    __write(IceInternal.BasicStream __os)
    {
        __os.startWriteObject(_slicedData);
        __os.endWriteObject();
    }

    @Override
    public void
    __read(IceInternal.BasicStream __is)
    {
        __is.startReadObject();
        _slicedData = __is.endReadObject(true);
    }

    private final String _unknownTypeId;
    private SlicedData _slicedData;
}
