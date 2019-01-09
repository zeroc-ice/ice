%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

function removeFolderFromPath(p)
  rmpath(p);
  savepath();
  exit(0);
end
