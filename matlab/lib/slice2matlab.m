function [status, out] = slice2matlab(args)

    % slice2matlab   Summary of slice2matlab
    %
    % A wrapper function to call Slice to MATLAB compiler slice2matlab.exe.
    %
    % Parameters:
    %   args - arguments passed to slice2matlab.exe.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    function path = findFile(candidates, baseDir, type)
        path = '';
        for i = 1:length(candidates)
            f = fullfile(baseDir, candidates{i});
            if exist(f, 'file') == type
                path = f;
                break;
            end
        end
    end

    rootDir = fileparts(mfilename('fullpath'));

    path = findFile({'..\slice2matlab.exe'
                     '..\..\cpp\bin\x64\Release\slice2matlab.exe'
                     '..\..\cpp\bin\x64\Debug\slice2matlab.exe'
                     '..\msbuild\packages\zeroc.ice.v140\build\native\tools\slice2matlab.exe'}, rootDir, 2);

    searchPath = findFile({'..\slice' '..\..\slice'}, rootDir, 7);

    if isempty(path)
        status = 1;
        out = 'Cannot locate slice2matlab.exe\n';
    elseif  isempty(searchPath)
        status = 1;
        out = 'Cannot locate slice dir.\n';
    else
        [status, out] = system(sprintf('%s -I%s %s', path, searchPath, args));
    end

    if ~isempty(out)
        fprintf('%s', out);
    end
end
