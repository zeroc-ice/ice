// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class BookFactory implements Ice.ObjectFactory
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
