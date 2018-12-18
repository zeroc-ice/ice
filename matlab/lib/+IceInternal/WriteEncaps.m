%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef WriteEncaps < handle
    properties
        start
        format = Ice.FormatType.DefaultFormat
        encoding
        encoder
        next
    end
end
