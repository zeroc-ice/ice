%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function buildToolbox(iceVersion)
  rootDir = fileparts(mfilename('fullpath'));
  packageDir = fullfile(rootDir, 'toolbox');
  sourceDir = fullfile(packageDir, 'ice');
  projectFile = fullfile(rootDir, 'toolbox.prj');
  try
    fprintf(1, 'Building Ice toolbox package... ');
    f = fopen(fullfile(sourceDir, 'Contents.m'), 'w+');
    fprintf(f, '%% ice\r\n');
    fprintf(f, '%% Version %s (R%s) %s\r\n', iceVersion, version('-release'), date);
    fprintf(f, '%%\r\n');
    fprintf(f, ['%% Please refer to the '...
                '<a href="https://doc.zeroc.com/display/Rel/Ice+3.7.3+Release+Notes">Ice Release Notes</a> '...
                'for more information\r\n']);
    fclose(f);
    matlab.addons.toolbox.packageToolbox(projectFile, sprintf('ice-%s-R%s.mltbx', iceVersion, version('-release')))
    fprintf(1, 'ok\r\n');
    exit(0);
  catch e
    fprintf(1, 'failed\r\n%s', getReport(e));
    exit(1);
  end
end
