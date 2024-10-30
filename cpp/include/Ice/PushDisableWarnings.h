// Copyright (c) ZeroC, Inc.

/*! \file
    \hideincludedbygraph
*/

// This file can be included several times in a translation unit.

#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4250) // ... : inherits ... via dominance
#    pragma warning(disable : 4251) // class ... needs to have dll-interface to be used by clients of class ...
#    pragma warning(disable : 4512) //  ... assignment operator could not be generated
#    pragma warning(disable : 4275) // non dll-interface class ... used as base for dll-interface class ...
#    pragma warning(disable : 4996) // ... was declared deprecated

#elif defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wredundant-decls"             // expected when using forward Slice declarations
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor" // expected in some generated header files
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"     // allow referencing deprecated Slice definitions
#    pragma clang diagnostic ignored "-Wdocumentation-deprecated-sync"

#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wredundant-decls"         // expected when using forward Slice declarations
#    pragma GCC diagnostic ignored "-Wshadow"                  // expected in some generated header files
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations" // allow referencing deprecated Slice definitions
#endif
