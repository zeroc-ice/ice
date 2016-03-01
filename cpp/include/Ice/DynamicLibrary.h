// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DYNAMIC_LIBRARY_H
#define ICE_DYNAMIC_LIBRARY_H

#include <Ice/DynamicLibraryF.h>
#include <IceUtil/Shared.h>

namespace IceInternal
{

class ICE_API DynamicLibrary : public ::IceUtil::Shared
{
public:

    DynamicLibrary();
    ~DynamicLibrary();

#ifdef _WIN32
    typedef FARPROC symbol_type;
#else
    typedef void* symbol_type;
#endif

    //
    // Load an entry point. This is really a convenience function
    // which combines calls to load() and getSymbol(). However, it
    // does add some value.
    //
    // An entry point has the following format:
    //
    // name[,version]:function
    //
    // The name of the library is constructed from the given
    // information. If no version is supplied and the boolean
    // argument is true, the Ice version (10 * major + minor) is 
    // used instead.
    //
    // For example, consider the following entry point:
    //  
    // foo:create 
    //
    // This would result in libfoo.so.11 (Unix) and foo11.dll
    // (Windows), where the Ice version is 1.1.x.
    //
    // Now consider this entry point:
    //
    // foo,12:create
    //
    // The library names in this case are libfoo.so.12 (Unix) and
    // foo12.dll (Windows).
    //
    // On Windows platforms, a 'd' is appended to the version for
    // debug builds.
    //
    // Returns 0 if a failure occurred.
    //
    symbol_type loadEntryPoint(const std::string&, bool = true);

    //
    // Open a library with the given path.
    //
    bool load(const std::string&);

    //
    // Retrieve a symbol from the library. Returns 0 if no match is found.
    //
    symbol_type getSymbol(const std::string&);

    //
    // Get the error message for the last failure.
    //
    const std::string& getErrorMessage() const;

private:

#ifdef _WIN32
    HINSTANCE _hnd;
#else
    void* _hnd;
#endif
    std::string _err;
};

class ICE_API DynamicLibraryList : public ::IceUtil::Shared
{
public:

    void add(const DynamicLibraryPtr&);

private:

    std::vector<DynamicLibraryPtr> _libraries;
};

}

#endif
