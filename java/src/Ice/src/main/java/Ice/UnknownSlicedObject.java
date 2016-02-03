// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
    __write(OutputStream __os)
    {
        __os.startObject(_slicedData);
        __os.endObject();
    }

    @Override
    public void
    __read(InputStream __is)
    {
        __is.startObject();
        _slicedData = __is.endObject(true);
    }

    private final String _unknownTypeId;
    private SlicedData _slicedData;
}
