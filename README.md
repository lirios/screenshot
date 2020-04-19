Screenshot
==========

[![License](https://img.shields.io/badge/license-GPLv3.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)
[![GitHub release](https://img.shields.io/github/release/lirios/screenshot.svg)](https://github.com/lirios/screenshot)
[![Build Status](https://travis-ci.org/lirios/screenshot.svg?branch=develop)](https://travis-ci.org/lirios/screenshot)
[![GitHub issues](https://img.shields.io/github/issues/lirios/screenshot.svg)](https://github.com/lirios/screenshot/issues)

Utility to capture the screen of a Liri desktop.

## Dependencies

Make sure you have Qt >= 5.10.0 with the following modules:

 * [qtbase](http://code.qt.io/cgit/qt/qtbase.git)
 * [qtdeclarative](http://code.qt.io/cgit/qt/qtdeclarative.git)
 * [qtquickcontrols2](http://code.qt.io/cgit/qt/qtquickcontrols2.git)

The following modules and their dependencies are required:

 * [cmake](https://gitlab.kitware.com/cmake/cmake) >= 3.10.0
 * [cmake-shared](https://github.com/lirios/cmake-shared.git) >= 1.0.0
 * [fluid](https://github.com/lirios/fluid.git)
 * [liri-wayland](https://github.com/lirios/wayland)

## Installation

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
make
make install # use sudo if necessary
```

Replace `/path/to/prefix` to your installation prefix.
Default is `/usr/local`.

## Licensing

Licensed under the terms of the GNU General Public License version 3.0 or,
at your option, any later version.
