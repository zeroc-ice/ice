// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

class DynamicLibrary : public ::IceUtil::Shared
{
public:

    DynamicLibrary();
    ~DynamicLibrary();

    bool load(const std::string&);

#ifdef WIN32
    typedef FARPROC symbol_type;
#else
    typedef void* symbol_type;
#endif

    symbol_type getSymbol(const std::string&); // Returns 0 if no match found

    const std::string& getErrorMessage() const; // Error message for last failure

private:

#ifdef WIN32
    HINSTANCE _hnd;
#else
    void* _hnd;
#endif

    std::string _err;
};

}

#endif
