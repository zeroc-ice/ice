// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    // which combines calls to load() and getSymbol(). However,
    // it does add some value.
    //
    // An entry point has the following format:
    //
    // name[,version]:function
    //
    // The name of the library is constructed from the given
    // information. If no version is supplied, the Ice version
    // is used. For example, consider the following entry point:
    //  
    // foo:create 
    //
    // This would result in libfoo.so.0.0.1 (Unix) and foo001.dll (Windows),
    // where the Ice version is 0.0.1.
    //
    // Now consider this entry point:
    //
    // foo,1.1:create
    //
    // The library names in this case are libfoo.so.1.1 (Unix) and
    // foo11.dll (Windows).
    //
    // On Windows platforms, a 'd' is appended to the version for debug
    // builds.
    //
    // Returns 0 if a failure occurred.
    //
    symbol_type loadEntryPoint(const std::string&);

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
