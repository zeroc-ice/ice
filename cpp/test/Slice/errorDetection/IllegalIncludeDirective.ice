// ice-fmt: ignore-file

#include "include/IcePrefix.ice" // okay

/*
This should be scanned as a comment, not preprocessor directives.
#include "FakeFile.ice"
#line 79 "Fake*File.ice"
*/

/**
This should be scanned as a doc-comment, not preprocessor directives.
#include "FakeFile.ice"
#line 79 "Fake*File.ice"
*/

module Definitions
{
    #include "include/IcePrefix.ice" // bad
}
