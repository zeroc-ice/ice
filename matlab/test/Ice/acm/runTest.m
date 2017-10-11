function runTest(varargin)
    try
        Client(varargin);
        exit(0);
    catch ex
        disp(getReport(ex, 'extended'));
        exit(1);
    end
end
