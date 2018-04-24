function removeFolderFromPath(p)
  rmpath(p);
  savepath();
  exit(0);
end
