// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_UNDEF_SYS_MACROS_H
#define ICE_UTIL_UNDEF_SYS_MACROS_H

//
// This header includes macros that can end up being dragged into
// the generated code from system headers, such as major() or NDEBUG.
// If a Slice symbol has the same name as a macro, the generated
// code most likely won't compile (but, depending how the macro is
// defined, may even compile).
//
// Here, we undefine symbols that cause such problems.
//
// The #ifdef ... #endif protection is necessary to prevent
// warnings on some platforms.
//

#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif
