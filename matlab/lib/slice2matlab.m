function status = slice2matlab(args)

    % SLICE2MATLAB Compile Slice files.
    %
    % Compiles Slice files using the Slice-to-MATLAB compiler
    % included with the Ice toolbox.
    %
    % Parameters:
    %
    %   args - The arguments passed to the Slice-to-MATLAB compiler,
    %          specified as a string.
    %
    % Returns:
    %
    %   status - The Slice-to-MATLAB compiler exit status code.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

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
    out = '';
    if isempty(path)
        status = 1;
        out = 'Cannot locate slice2matlab.exe\n';
    elseif  isempty(searchPath)
        status = 1;
        out = 'Cannot locate slice dir.\n';
    else
        status = system(sprintf('"%s" -I"%s" %s', path, searchPath, args));
    end

    if ~isempty(out)
        fprintf('%s', out);
    end
end
