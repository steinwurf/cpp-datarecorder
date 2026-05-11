=============
Data Recorder
=============

Overview
========

datarecorder is a simple C++ library that is meant to provide simple helper
functions to ease sometimes tedious testing tasks.

Build and test
==============

Configure::

    ./waf configure

Build::

    ./waf build

Run tests::

    ./waf build --run_tests

Mismatch template
=================

By default, datarecorder will try to find this template:

- ``visualizer/recording_diff.html``

You can override this with::

    recorder.set_mismatch_template("path/to/recording_diff.html");

If the provided path is relative, datarecorder resolves it by searching
backwards from the current working directory.

If you provide a custom mismatch template path and it cannot be found,
recording will fail.

Template requirements
---------------------

The template must contain both of these JavaScript markers (using backticks)::

    const oldText = `...`;
    const newText = `...`;

These placeholders are replaced with recording data and mismatch data.

License
-------
The stub library is released under the BSD license, see the LICENSE.rst file.
