%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function removeFolderFromPath(p)
  try
    fprintf(1, 'Removing toolxbox folder from path... ');
    rmpath(p);
    savepath();
    fprintf(1, 'ok\n');
    exit(0);
  catch e
    fprintf(1, 'failed\n%s', getReport(e));
    exit(1);
  end
end
