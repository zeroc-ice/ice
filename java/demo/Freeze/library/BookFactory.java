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
	return new BookI(_library, _evictor);
    }

    public void
    destroy()
    {
    }

    BookFactory(LibraryI library, Freeze.Evictor evictor)
    {
	_library = library;
	_evictor = evictor;
    }

    private LibraryI _library;
    private Freeze.Evictor _evictor;
}
