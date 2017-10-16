%
% This function runs a MATLAB test client. The first argument must the path name of the test directory. All
% other arguments are passed to the test client.
%
function runTest(varargin)
    if length(varargin) == 0
        fprintf('testdir argument required\n');
        exit(1);
    end

    testdir = varargin{1};
    varargin(1) = []; % Removes first argument.

    cd(testdir);

    try
        client(varargin);
        exit(0);
    catch ex
        disp(getReport(ex, 'extended'));
        exit(1);
    end
end
