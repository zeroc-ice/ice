%{
**********************************************************************
Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.
**********************************************************************
%}
function [status, out] = slice2matlab(args)
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
