// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
