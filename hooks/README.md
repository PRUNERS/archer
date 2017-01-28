<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#orgd1eb8a8">1. How to setup the hooks</a></li>
<li><a href="#orgc0249b8">2. Prerequisites</a>
<ul>
<li><a href="#orgcb36cae">2.1. Pre-Commit Hook</a></li>
<li><a href="#orge67a783">2.2. Post-Commit Hook</a></li>
</ul>
</li>
</ul>
</div>
</div>


<a id="orgd1eb8a8"></a>

# How to setup the hooks

These hooks allow to automatically generate the Markdown README from
the Org-Mode README.


<a id="orgc0249b8"></a>

# Prerequisites

In order to generate the Markdown README from the Org-Mode README you
need a recent version of Emacs (>= 24.4) and the "org" package
installed.

To install the "org" package open Emacs, type M-x list-packages to
open the package list. Look for "org" package, press ‘i’ to mark for
installation, and ‘x’ to perform the installation.

From the root folder of the git repository run:


<a id="orgcb36cae"></a>

## Pre-Commit Hook

    ln -s -f ../../hooks/pre-commit .git/hooks/post-pre


<a id="orge67a783"></a>

## Post-Commit Hook

    ln -s -f ../../hooks/pre-commit .git/hooks/post-pre
