// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <IceUtil/Config.h>

#ifndef SLICE_API
#   ifdef SLICE_API_EXPORTS
#       define SLICE_API ICE_DECLSPEC_EXPORT
#   else
#       define SLICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Slice
{

class SLICE_API Preprocessor
{
public:

    Preprocessor(const std::string&, const std::string&, const std::string&);
    ~Preprocessor();
    
    FILE* preprocess(bool);
    bool close();

    void printMakefileDependencies();
    
    std::string getBaseName();

private:
    
    bool checkInputFile();
    std::string searchIceCpp();

    const std::string _path;
    const std::string _fileName;
    const std::string _args;
    FILE* _cppHandle;
};

}

#endif
