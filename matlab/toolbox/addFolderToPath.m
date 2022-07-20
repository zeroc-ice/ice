%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function addFolderToPath(f)
  try
    fprintf(1, 'Adding toolxbox folder to path... ');
    addpath(f);
    savepath();
    fprintf(1, 'ok\n');
    exit(0);
  catch e
    fprintf(1, 'failed\n%s', getReport(e));
    exit(1);
  end
end
