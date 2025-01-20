# Copyright (c) ZeroC, Inc.

module Ice

    # Convenience function for locating the directory containing the Slice files.
    def Ice.getSliceDir

        # Get the grand-parent of the directory containing this file (Ice/SliceUtil.rb).
        rbHome = File::join(File::dirname(__FILE__), "../..")

        # For an installation from a source distribution, a binary tarball, the "slice" directory is a sibling of the
        # "ruby" directory.
        dir = File::join(rbHome, "slice")
        if File::exist?(dir)
            return File::expand_path(dir)
        end

        # In a source distribution, the "slice" directory is one level higher.
        dir = File::join(rbHome, "..", "slice")
        if File::exist?(dir)
            return File::expand_path(dir)
        end

        # Gem distribution
        dir = File::join(rbHome, "..", "dist", "ice", "slice")
        if File::exist?(dir)
            return File::expand_path(dir)
        end

        if RUBY_PLATFORM =~ /linux/i

            # Check the default Linux location.
            dir = File::join("/", "usr", "share", "ice", "slice")
            if File::exist?(dir)
                return dir
            end
        end

        return nil
    end
end
