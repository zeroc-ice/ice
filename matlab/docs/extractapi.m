function extractapi(outputFile)
    %EXTRACTAPI Extract the public API of Ice for MATLAB into a JSON file.
    %   Walks the public packages with the metaclass API and captures the help text of each documented symbol into
    %   matlab-api.json. The companion script renderapi.py renders this file into a static HTML API reference; nothing
    %   in the rendered output requires MATLAB.
    %
    %   Both lib and lib/generated must be on the MATLAB path (this function adds them), and lib/generated must be
    %   populated by a prior build.

    % Copyright (c) ZeroC, Inc.

    if nargin < 1
        outputFile = fullfile(fileparts(mfilename('fullpath')), 'matlab-api.json');
    end

    % Keep the <a href="matlab:help ..."> anchors emitted by slice2matlab in the captured help text.
    feature('hotlinks', 1);

    rootDir = fileparts(fileparts(mfilename('fullpath')));
    addpath(fullfile(rootDir, 'lib'));
    addpath(fullfile(rootDir, 'lib', 'generated'));

    api.schemaVersion = 1;
    api.matlabRelease = ['R', version('-release')];
    api.functions = {extractFunction('slice2matlab')};

    packageNames = {'Ice', 'Ice.SSL', 'Glacier2', 'IceBox', 'IceGrid', 'IceMX', 'IceStorm'};
    packages = cell(1, numel(packageNames));
    for i = 1:numel(packageNames)
        packages{i} = extractPackage(packageNames{i});
    end
    api.packages = packages;

    checkPackage(api, 'Ice', 50);
    checkPackage(api, 'Glacier2', 1);
    checkPackage(api, 'IceBox', 1);
    checkPackage(api, 'IceGrid', 1);
    checkPackage(api, 'IceMX', 1);
    checkPackage(api, 'IceStorm', 1);

    text = jsonencode(api, PrettyPrint = true);
    fid = fopen(outputFile, 'w', 'n', 'UTF-8');
    if fid == -1
        error('extractapi:cannotOpen', 'Cannot open %s for writing.', outputFile);
    end
    fprintf(fid, '%s\n', text);
    fclose(fid);
    fprintf('Wrote %s\n', outputFile);
end

function checkPackage(api, name, minClasses)
    % A package with fewer classes than expected means the generated code is missing; fail instead of publishing a
    % gutted reference.
    for i = 1:numel(api.packages)
        if strcmp(api.packages{i}.name, name)
            if numel(api.packages{i}.classes) < minClasses
                error('extractapi:incompletePackage', ...
                      'Package %s has %d classes, expected at least %d. Is lib/generated populated?', ...
                      name, numel(api.packages{i}.classes), minClasses);
            end
            return;
        end
    end
    error('extractapi:missingPackage', 'Package %s was not extracted.', name);
end

function pkg = extractPackage(name)
    mp = meta.package.fromName(name);
    if isempty(mp)
        error('extractapi:missingPackage', 'Package %s not found on the MATLAB path.', name);
    end

    pkg.name = name;

    classes = {};
    for i = 1:numel(mp.ClassList)
        mc = mp.ClassList(i);
        if mc.Hidden
            continue;
        end
        classes{end + 1} = extractClass(mc); %#ok<AGROW>
    end
    pkg.classes = classes;

    functions = {};
    for i = 1:numel(mp.FunctionList)
        functions{end + 1} = extractFunction([name, '.', mp.FunctionList(i).Name]); %#ok<AGROW>
    end
    pkg.functions = functions;
end

function cls = extractClass(mc)
    cls.name = mc.Name;
    if mc.Enumeration
        cls.kind = 'enum';
    else
        cls.kind = 'class';
    end
    cls.abstract = mc.Abstract;

    superclasses = {};
    for i = 1:numel(mc.SuperclassList)
        superclasses{end + 1} = mc.SuperclassList(i).Name; %#ok<AGROW>
    end
    cls.superclasses = superclasses;
    cls.help = getHelp(mc.Name);

    [~, shortName] = splitName(mc.Name);

    constructor = [];
    methodList = {};
    for i = 1:numel(mc.MethodList)
        m = mc.MethodList(i);
        if m.Hidden || ~strcmp(accessName(m.Access), 'public') || isForeignClass(m.DefiningClass.Name)
            continue;
        end
        entry = extractMethod(mc, m);
        if strcmp(m.Name, shortName)
            constructor = entry;
        else
            methodList{end + 1} = entry; %#ok<AGROW>
        end
    end
    if mc.Enumeration
        % Enumerations are not constructed by users; their synthesized constructor is not part of the API.
        constructor = [];
    elseif ~isempty(constructor) && strcmp(constructor.help, cls.help)
        % help() falls back to the class help when the constructor has no help block of its own.
        constructor.help = '';
    end
    cls.constructor = constructor;
    cls.methods = methodList;

    propertyList = {};
    for i = 1:numel(mc.PropertyList)
        p = mc.PropertyList(i);
        if p.Hidden || ~strcmp(accessName(p.GetAccess), 'public') || isForeignClass(p.DefiningClass.Name)
            continue;
        end
        propertyList{end + 1} = extractProperty(mc, p); %#ok<AGROW>
    end
    cls.properties = propertyList;

    members = {};
    for i = 1:numel(mc.EnumerationMemberList)
        em = mc.EnumerationMemberList(i);
        entry.name = em.Name;
        entry.help = getHelp([mc.Name, '.', em.Name]);
        % Ice enumerations subclass int32 or uint8, so evaluating a member executes no user code.
        entry.value = double(eval([mc.Name, '.', em.Name]));
        members{end + 1} = entry; %#ok<AGROW>
    end
    cls.enumerationMembers = members;
end

function entry = extractMethod(mc, m)
    entry.name = m.Name;
    entry.definingClass = m.DefiningClass.Name;
    entry.static = m.Static;
    entry.abstract = m.Abstract;
    entry.inputs = cellstr(m.InputNames);
    entry.outputs = cellstr(m.OutputNames);
    if strcmp(m.DefiningClass.Name, mc.Name)
        % Inherited methods get no help text; the renderer links to the defining class instead.
        entry.help = getHelp([mc.Name, '/', m.Name]);
    else
        entry.help = '';
    end
end

function entry = extractProperty(mc, p)
    % Deliberately never reads p.DefaultValue: querying it evaluates the initializer expression. The type and default
    % are already described by the help comment's type line.
    entry.name = p.Name;
    entry.definingClass = p.DefiningClass.Name;
    entry.constant = p.Constant;
    entry.dependent = p.Dependent;
    entry.setAccess = accessName(p.SetAccess);
    if strcmp(p.DefiningClass.Name, mc.Name)
        entry.help = getHelp([mc.Name, '/', p.Name]);
    else
        entry.help = '';
    end
end

function f = extractFunction(name)
    f.name = name;
    f.help = getHelp(name);
    f.declaration = readDeclaration(name);
end

function declaration = readDeclaration(name)
    % The function metadata does not reliably expose signatures, so read the declaration from the source file.
    declaration = '';
    file = which(name);
    if isempty(file) || ~isfile(file)
        return;
    end
    lines = readlines(file);
    for i = 1:numel(lines)
        line = strtrim(lines(i));
        if startsWith(line, 'function')
            % Strip a trailing comment or line continuation from the declaration.
            declaration = char(strip(regexprep(line, '\s*(%|\.\.\.).*$', '')));
            return;
        end
    end
end

function name = accessName(access)
    % Access is a char vector, or a cell array of metaclasses for access lists; the latter is not public.
    if ischar(access) || isstring(access)
        name = char(access);
    else
        name = 'restricted';
    end
end

function r = isForeignClass(name)
    % Members defined by MATLAB base classes are boilerplate (addlistener, eq, ...) and get no page in the reference.
    [packageName, ~] = splitName(name);
    r = isempty(packageName) || strcmp(packageName, 'matlab') || startsWith(name, 'matlab.');
end

function [packageName, shortName] = splitName(name)
    pos = find(name == '.', 1, 'last');
    if isempty(pos)
        packageName = '';
        shortName = name;
    else
        packageName = name(1:pos - 1);
        shortName = name(pos + 1:end);
    end
end

function text = getHelp(name)
    try
        text = help(name);
    catch
        text = '';
    end
    if isempty(text)
        return;
    end

    % Strip the footer lines that the help command appends after the help comment itself, for example:
    %   Documentation for Ice.Communicator
    %      doc Ice.Communicator
    lines = splitlines(text);
    footers = ["Documentation for ", "doc ", "Folders named ", "Other uses of ", "Other functions named "];
    for i = 1:numel(lines)
        if startsWith(strip(eraseAnchors(lines{i})), footers)
            lines = lines(1:i - 1);
            break;
        end
    end

    % Remove one level of indentation: help() indents every line of the comment block.
    for i = 1:numel(lines)
        line = lines{i};
        if startsWith(line, ' ')
            lines{i} = line(2:end);
        end
    end
    text = char(strip(strjoin(lines, newline), 'right'));
end

function text = eraseAnchors(text)
    text = regexprep(text, '<a\s+href="[^"]*">|</a>', '');
end
