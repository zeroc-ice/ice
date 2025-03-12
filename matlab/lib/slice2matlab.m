function status = slice2matlab(varargin)

    % SLICE2MATLAB Compile Slice files.
    %
    % Compiles Slice files using the Slice-to-MATLAB compiler included with the Ice toolbox.
    %
    % Parameters:
    %
    %   varargin - The arguments passed to the Slice-to-MATLAB compiler.
    %
    % Returns:
    %
    %   status - The Slice-to-MATLAB compiler exit status code.

    % Copyright (c) ZeroC, Inc.

    root_dir = fileparts(mfilename('fullpath'));

    % Paths to search for the slice2matlab compiler.
    exe_paths = {
        fullfile(root_dir, 'slice2matlab'),
        fullfile(root_dir, 'slice2matlab.exe'),
        fullfile(root_dir, '..', '..', 'cpp', 'bin', 'x64', 'Release', 'slice2matlab.exe'),
        fullfile(root_dir, '..', '..', 'cpp', 'bin', 'x64', 'Debug', 'slice2matlab.exe'),
        fullfile(root_dir, '..', '..', 'cpp', 'bin', 'slice2matlab')
    };

    % Find the first candidate in exe_paths that is a file.
    idx = find(cellfun(@isfile, exe_paths), 1);

    if ~isempty(idx)
        path = exe_paths{idx};
    else
        error('Cannot locate slice2matlab compiler');
    end

    search_paths = {
        fullfile(root_dir, 'slice'),
        fullfile(root_dir, '..', '..', 'slice'),
    };

    % Find the first path in search_paths that is a folder.
    idx = find(cellfun(@isfolder, search_paths), 1);

    if ~isempty(idx)
        search_path = search_paths{idx};
    else
        error('Cannot locate slice dir.');
    end

    status = system(sprintf('"%s" -I"%s" %s', path, search_path, strjoin(varargin, ' ')));
end
