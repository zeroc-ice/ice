// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class BookFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
	assert(type.equals("::Book"));
	return new BookI(_library);
    }

    public void
    destroy()
    {
    }

    BookFactory(LibraryI library)
    {
	_library = library;
    }

    private LibraryI _library;
}
