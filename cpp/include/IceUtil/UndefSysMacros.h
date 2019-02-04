//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
