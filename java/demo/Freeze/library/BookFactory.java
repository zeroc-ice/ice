// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class BookFactory implements Ice.ObjectFactory
{
    @Override
    public Ice.Object
    create(String type)
    {
        assert(type.equals("::Demo::Book"));
        return new BookI(_library);
    }

    @Override
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
