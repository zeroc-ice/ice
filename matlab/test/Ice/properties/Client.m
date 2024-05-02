%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function client(args)
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    props = Ice.createProperties()

    fprintf('testing ice properties with set default values...');
    toStringMode = props.getIceProperty('Ice.ToStringMode');
    assert(strcmp(toStringMode, 'Unicode'));
    closeTimeout = props.getIcePropertyAsInt('Ice.Connection.CloseTimeout');
    assert(closeTimeout == 10);
    retryIntervals = props.getIcePropertyAsList('Ice.RetryIntervals');
    assert(length(retryIntervals) == 1);
    assert(strcmp(retryIntervals{1}, '0'));
    fprintf('ok\n');

    fprintf('testing ice properties with unset default values...');
    stringValue = props.getIceProperty('Ice.Admin.Router');
    assert(strcmp(stringValue, ''));
    intValue = props.getIcePropertyAsInt('Ice.Admin.Router');
    assert(intValue == 0);
    listValue = props.getIcePropertyAsList('Ice.Admin.Router');
    assert(length(listValue) == 0);
    fprintf('ok\n');

    fprintf('testing that getting an unknown ice property throws an exception...');
    try
        props.getIceProperty('Ice.UnknownProperty');
        assert(false);
    catch ex
    end
    fprintf('ok\n');

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
