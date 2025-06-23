function mustBeStringStringDictionary(dict)
    %MUSTBESTRINGSTRINGDICTIONARY Verify dict is a string-string dictionary.
    %
    %   Input Arguments
    %     dict - The dictionary to check.
    %       dictionary scalar

    % Copyright (c) ZeroC, Inc.

    arguments
        dict (1, 1) dictionary
    end

    [keyType, valueType] = dict.types;
    if keyType ~= "string" || valueType ~= "string"
        error('Ice:ArgumentException', 'The dictionary must have string keys and string values.');
    end
end
