%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

function addFolderToPath(f)
  addpath(f);
  savepath();
  exit(0);
end
