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

    % Copyright (c) ZeroC, Inc.

    rootDir = fileparts(mfilename('fullpath'));

    slice2matlab_paths = {
        fullfile(rootDir, 'slice2matlab'),
        fullfile(rootDir, 'slice2matlab.exe'),
        fullfile(rootDir, '..', '..', 'cpp', 'bin', 'x64', 'Release', 'slice2matlab.exe'),
        fullfile(rootDir, '..', '..', 'cpp', 'bin', 'x64', 'Debug', 'slice2matlab.exe'),
        fullfile(rootDir, '..', 'msbuild', 'packages', 'zeroc.ice.v143', 'build', 'native', 'tools', 'slice2matlab.exe')
    };

    % Find the first candidate in slice2matlab_paths that is a file.
    idx = find(cellfun(@isfile, slice2matlab_paths), 1);

    if ~isempty(idx)
        path = slice2matlab_paths{idx};
    else
        path = '';  % or handle the “no match” case appropriately
    end

    slice_paths = {
        fullfile(rootDir, 'slice'),
        fullfile(rootDir, '..', '..', 'slice'),
    };

    % Find the first path in slice_paths that is a folder.
    idx = find(cellfun(@isfolder, slice_paths), 1);

    if ~isempty(idx)
        search_path = slice_paths{idx};
    else
        search_path = '';  % or handle the “no match” case appropriately
    end

    if isempty(path)
        status = 1;
        fprintf('\nerror: Cannot locate slice2matlab compiler\n');
    elseif isempty(search_path)
        status = 1;
        fprintf('\nerror: Cannot locate slice dir.\n');
    else
        status = system(sprintf('"%s" -I"%s" %s', path, search_path, args));
    end
end
