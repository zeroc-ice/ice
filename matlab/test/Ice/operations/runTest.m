function runTest(varargin)
    try
        Client(varargin);
        exit(0);
    catch ex
        exit(1);
    end
end
