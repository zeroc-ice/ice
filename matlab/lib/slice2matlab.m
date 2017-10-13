function [status, out] = slice2matlab(args)
    % slice2matlab   Summary of slice2matlab
    %
    % A wrapper function to call Slice to MATLAB compiler slice2matlab.exe.
    %
    % Parameters:
    %   args - arguments passed to slice2matlab.exe.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    rootDir = fileparts(mfilename('fullpath'));

    candidates = {'..\slice2matlab.exe'
                  '..\..\cpp\bin\x64\Release\slice2matlab.exe'
                  '..\..\cpp\bin\x64\Debug\slice2matlab.exe'
                  '..\msbuild\packages\zeroc.ice.v140\build\native\tools\slice2matlab.exe'};

    path = '';
    for i = 1:length(candidates)
        f = fullfile(rootDir, candidates{i});
        if exist(f, 'file') == 2
            path = f;
            break;
        end
    end

    if isempty(path)
        status = 1;
        out = 'Cannot locate slice2matlab.exe\n';
    else
        [status, out] = system(sprintf('%s %s', path, args));
    end

    if ~isempty(out)
        fprintf(1, '%s', out);
    end
end
