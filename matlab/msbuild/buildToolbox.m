function buildToolbox(iceVersion)
  rootDir = fileparts(mfilename('fullpath'));
  packageDir = fullfile(rootDir, 'toolbox');
  sourceDir = fullfile(packageDir, 'ice');
  projectFile = fullfile(rootDir, 'ice.prj');
  oldPath = path();
  path(oldPath, sourceDir);
  try
    fprintf(1, 'Building Ice toolbox package... ');
    f = fopen(fullfile(sourceDir, 'Contents.m'), 'w+');
    fprintf(f, '%% ice\r\n');
    fprintf(f, '%% Version %s (R%s) %s\r\n', iceVersion, version('-release'), date);
    fclose(f);
    matlab.addons.toolbox.packageToolbox(projectFile);
    path(oldPath);
    fprintf(1, 'ok\n');
  catch e
    path(oldPath);
    fprintf(1, 'failed\n');
    e.rethrow();
  end
end
