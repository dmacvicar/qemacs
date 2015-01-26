
# An unamed QEmacs fork

## About QEmacs

QEmacs is an Emacs clone created by Fabrice Bellard.
Since 2004, it was maintained by Charlie Gordon until early 2014.

## About this fork

This fork has educational purposes and the following goals in addition
to general refactorings and cleanups:

- [x] a Qt frontend
- [x] build with CMake
- [ ] remove unecesary stuff (video and others)
- [ ] add scripting support (lua?)

# Compiling

```
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -DCONFIG_QT=ON ..
make
```

## Some caveats

* The original qe supported x11 and terminal in one binary. The Qt
  version uses the Qt event loop and that conflicts with unix.c event
  loop used by the terminal one. We need a separate binary like the
  real emacs-nox.

## Documentation

Read the file qe-doc.html.

## Licensing

QEmacs is released under the GNU Lesser General Public License (read
the accompagning COPYING file).
