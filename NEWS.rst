News for datarecorder
=====================

This file lists the major changes between versions. For a more detailed list of
every change, see the Git log.

Latest
------
* Minor: Added ``set_mismatch_template()`` to configure the mismatch HTML template path.
* Minor: Custom mismatch template paths now fail if they cannot be resolved.
* Minor: Added template format validation for ``oldText`` and ``newText`` markers.

2.0.0
-----
* Major: Update poke to v17.

1.0.2
-----
* Patch: Stopped the creation of mismatch dir when no files will be written to it.

1.0.1
-----
* Patch: Fix infinite loop on Windows when looking for recording directory.

1.0.0
-----
* Major: Initial release.
