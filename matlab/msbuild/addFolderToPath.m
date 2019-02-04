%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function addFolderToPath(f)
  addpath(f);
  savepath();
  exit(0);
end
