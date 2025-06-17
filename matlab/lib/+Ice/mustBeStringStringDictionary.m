% Copyright (c) ZeroC, Inc.

function mustBeStringStringDictionary(dict)
    % Verify dict (a known dictionary) is a string-string dictionary.
    [keyType, valueType] = dict.types;
    if keyType ~= "string" || valueType ~= "string"
        error('Ice:InvalidDictionary', 'The dictionary must have string keys and string values.');
    end
end
