% Copyright (c) ZeroC, Inc.

function buildToolbox(iceVersion)
  rootDir = fileparts(mfilename('fullpath'));
  packageDir = fullfile(rootDir, 'build');
  projectFile = fullfile(rootDir, 'toolbox.prj');
  if isunix
      os = 'linux';
  else
    os = 'win';
  end

  try
    fprintf(1, 'Building Ice toolbox package... ');
    f = fopen(fullfile(packageDir, 'Contents.m'), 'w+');
    fprintf(f, '%% ice\r\n');
    fprintf(f, '%% Version %s (R%s) %s\r\n', iceVersion, version('-release'), date);
    fprintf(f, '%%\r\n');
    fprintf(f, ['%% Please refer to the '...
                '<a href="https://docs.zeroc.com/ice/3.8/cpp/ice-3-8-0">Ice Release Notes</a> '...
                'for more information\r\n']);
    fclose(f);
    matlab.addons.toolbox.packageToolbox(projectFile, sprintf('ice-%s-R%s-%s.mltbx', iceVersion, version('-release'), os))
    fprintf(1, 'ok\r\n');
    exit(0);
  catch e
    fprintf(1, 'failed\r\n%s', getReport(e));
    exit(1);
  end
end
