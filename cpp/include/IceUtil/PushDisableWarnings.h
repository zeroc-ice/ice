//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// No pragma once as this file can be included several times in a translation
// unit

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4250) // ... : inherits ... via dominance
#   pragma warning(disable:4251) // class ... needs to have dll-interface to be used by clients of class ...
#   pragma warning(disable:4512) //  ... assignment operator could not be generated
#   pragma warning(disable:4275) // non dll-interface class ... used as base for dll-interface class ...
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wredundant-decls" // expected when using forward Slice declarations
#   pragma clang diagnostic ignored "-Wdocumentation-deprecated-sync" // see zeroc-ice/ice issue #211

#   if (__clang_major__ >= 4)
#       pragma clang diagnostic ignored "-Wshadow-field-in-constructor" // expected in some generated header files
#   else
#       pragma clang diagnostic ignored "-Wshadow-all" // expected in some generated header files
#   endif

#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wredundant-decls" // expected when using forward Slice declarations
#   pragma GCC diagnostic ignored "-Wshadow" // expected in some generated header files
#endif
