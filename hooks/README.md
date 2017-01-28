# How to setup the hooks

These hooks allow to automatically generate the Markdown README from
the Org-Mode README.

# Prerequisites

In order to generate the Markdown README from the Org-Mode README you
need a recent version of Emacs (>= 24.4) and the "org" package
installed.

To install the "org" package open Emacs, type M-x list-packages to
open the package list. Look for "org" package, press ‘i’ to mark for
installation, and ‘x’ to perform the installation.

From the root folder of the git repository run:

## Pre-Commit Hook

#+BEGIN_SRC bash :exports code
ln -s -f ../../hooks/pre-commit .git/hooks/post-pre
#+END_SRC

## Post-Commit Hook

#+BEGIN_SRC bash :exports code
ln -s -f ../../hooks/pre-commit .git/hooks/post-pre
#+END_SRC
