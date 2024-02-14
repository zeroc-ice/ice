//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_DYNAMIC_LIBRARY_F_H
#define ICE_DYNAMIC_LIBRARY_F_H

#include <memory>

namespace IceInternal
{

class DynamicLibrary;
using DynamicLibraryPtr = std::shared_ptr<DynamicLibrary>;

class DynamicLibraryList;
using DynamicLibraryListPtr = std::shared_ptr<DynamicLibraryList>;

}
#endif
