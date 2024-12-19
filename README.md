# epub2txt -- Extract text from EPUB documents  

Version 2.11, December 2024 

## What is this? 

`epub2txt` is a simple command-line utility for extracting text from EPUB
documents and, optionally, re-flowing it to fit a text display of a particular
number of columns. It is written entirely in ANSI-standard C, and should run on
any Unix-like system with a C compiler. It is intended for reading EPUB e-books
on embedded systems that can't host a graphical EPUB viewer, or converting such
e-books to read on those systems.  However, it should be robust enough for
other purposes, such as batch indexing of EPUB document collections.

`epub2txt` favours speed and low memory usage over accuracy of rendering. Most
of the formatting of the source document will be lost but, with a text-only
display, this is likely to be of little consequence.

This utility is specifically written to have no dependencies on external
libraries, except the standard C library, and even on this it makes few
demands. It does expect to be able to run an "unzip" command, however. The
purpose of minimizing dependencies is to allow the utility to build on embedded
systems without needing to build a bunch of dependencies first.

`epub2txt` will output UTF8-encoded text by default, but can be told to output
ASCII, in which case it will try to convert non-ASCII characters into something
displayable if possible.

## Differences from epub2txt version 1.x 

`epub2txt` version 2.0 is a more-or-less complete reimplementation, compared to
the earlier 1.x releases. Not only has the internal logic been changed to
improve multi-byte character support, but the command-line switches have been
updated, to make the utility easier to use in the more common scenarios. Some
features from 1.x have been omitted in this new version, since they added
complexity and did not seem to be used much. 

* All character processing is now done using 32-bit, rather than 8-bit,
  characters, so each character requires exactly 32 bits. This makes formatting
  text that contains non-English characters much easier and, hopefully, more
  accurate, since the program no longer has to do complicated fiddling with
  multi-byte characters
* Where the source document uses "simple" formatting tags, like `<h1>` for
  headings and  and `<b>` for bold, `epub2txt` will output ANSI highlighting
  characters if the program is run from a terminal. This feature can be turned
  off, if required, but will be used by default 
* `epub2txt` tries to determine the actual width of the terminal and, in most
  cases, the user should not have to specify it
* It is assumed that output should be formatted to fit the width of a terminal,
  whenever a the program is run from a terminal. To override this behaviour,
  specify a width of zero (`-w 0`)
* There is a new `--raw` switch that does essentially what `--notrim -w 0` did
  in the previous version, and also implies
  no ANSI highlighting (`--noansi`).


## Prerequisites 

`epub2txt` is intended to run on Linux and other Unix-like systems. It makes
use of the common Unix `unzip` utility but has no other dependencies.  It
builds and runs on Windows under Cygwin, and under the Windows 10 Linux
subsystem (WSL), but not as a native Windows console application.  The system
must be set up such that there is a temporary directory at `/tmp` that users
can write to, unless the environment variable `TMP` is set, in which case that
will be used instead. 

## Building and installing 

`epub2txt` is already available for a number of Linux distributions, but to get
the latest version it is usually best to build from source. This should be
straightforward if `gcc` and `make` are installed.  All you should need to do
is

    $ make
    $ sudo make install


## Command-line switches 

For a full list, run `epub2txt --help`.

`-a, --asiii`

Reduces multi-byte characters to 7-bit ASCII if possible. Some very common
characters are easily converted, like the various Unicode spaces, which can be
converted into plain ASCII spaces. Common accented characters (.e.,g "&eacute;"
are converted -- for better or worse -- into their non-accented equivalents.
Some single-character entities like &copy; can be converted into
multi-character equivalents, like "(c)". What `epub2txt` <i>won't</i> do is to
convert multi-byte characters into single-byte characters in some form of
"extended ASCII" character set. Those days are gone, and I'm not going to help
bring them back. In any case, they're unlikely to display properly in a Linux
terminal.

`-n, --noansi`

Don't output ANSI terminal highlights. If `epub2txt` is run from a console, it
will interpret common HTML formatting in the source document (such as `<h1>`
for headings and  `<b>` for bold) by outputting ANSI highlight characters. Most
(all?) Linux terminals understand these characters, and render the text with
some sort of emphasis.  In practice, most EPUB authors and converters don't use
simple HTML markup (more's the pity), and even simple italic emphasis often
uses custom style classes. So in many cases, no ANSI highlights will be seen.
Moreover, some text processing utilities, like the common `more`, don't handle
them properly. In such cases, use `--noansi` to switch this feature off.

`-r, --raw`

Don't process text data in any way -- just dump paragraphs of text exactly as
they appear in the source document. Because some XHTML tags effectively create
a paragraph break, without actually using explicit paragraph divisions,
`epub2txt` will output a newline at the end of every such tag when it appears
in the EPUB document. Without this treatment, many documents would render as
one enormous line of text. However, sequences of empty lines might appear in
the output.

`-s, --separator=text`

Writes the specified text on its own line before decoding each spine item. This
option is intended for users who parse the output of `epub2txt` using scripts,
and want to split the output up into chapters. A parser can detect the special
text, and start a new chapters. 

Users should be aware, however, that there is no mandatory connection between
spine items and chapters. The effect of the `--separator` option will depend on
the software used to author the EPUB.

`-w, --width=N`

Format the output for a display with N columns. If either the standard input or
standard output of the `epub2txt` program is a terminal, the program will try
to work out how wide it is. If it can't, it will assume 80 characters. The
implication of using standard input to determine terminal width is that
`epub2txt` still assumes it must produce fixed-width output, even if output is
redirected to some other utility. This makes it possible to use `epub2txt`
without specific command-line switches in common modes of operation like:

    $ epub2txt myfile.epub | more

The `more` utility cannot wrap lines neatly on its own, so disabling line
wrapping when `stdout` is redirected would create additional work for the user. 

To turn off line wrapping specify `-w 0`, or `--raw`.  The difference between
these modes is that `-w 0` still collapses whitespace and multiple blank lines,
whilst `--raw` just outputs all text in the document, exactly as presented.

## Hints 

_Make a list of all unique words in an EPUB file, for indexing purposes:_

    $ epub2txt  --raw file.epub |tr -cs A-Za-z\' '\n' |  tr A-Z a-z | sort| uniq

Using `--raw` here speeds things up, as there is no need for `epub2txt` to
format the output, if it is just going to be used to make a word list.

_Read an EPUB on screen, with left justification_, using ANSI highlight codes
for headings, etc., if the document uses simple formatting tags:

    $ epub2txt file.epub | less -RS 

This is a convenient way to read an EPUB document when a graphical viewer is
not available. The `-R` switch to `less` tells it to respect ANSI highlight
characters, which it can usually do without losing track of how much text is on
a line.

_Read an EPUB on screen with full (left and right) justification:_

    $ epub2txt file.epub --noansi -w 0 | groff -K utf8 -Tascii | less 

Note that `groff` can't handle ANSI terminal highlight characters as input, so
these need to be disabled.


## Bugs and limitations 

The main development priorities for `epub2txt` are speed and compactness, not
military-grade security.  The program is not designed for use in hostile
environments, such as processing input from the Internet.  It's almost
certainly possible to craft an EPUB file that will cause a buffer overrun or
stack collision, with uncertain results. I don't want to bloat the utility
further by adding checks for every conceivable failure that a hostile EPUB
might manifest.  Please don't use `epub2txt` for applications where security is
a primary concern -- it's not designed for this, and is really not suitable. 

There is no support for any form of DRM or encryption, and such support
is unlikely to be added in the future.

`epub2txt` only handles documents that use UTF8 (or ASCII) encoding (but I
believe that UTF8 is more-or-less universal in EPUB), and writes output only in
UTF8 encoding, regardless of the platform's locale. This limitation exists
because `epub2txt` does all its own character encoding conversions to avoid
creating a dependency on an external library. Doing this for UTF8 is enough
work on its own; doing it for arbitrary encodings would be overwhelming. 

The program can't correct errors in encoding, and there are many EPUB documents
in public repositories that contain encoding errors.  A common problem is
spurious use of non-UTF8 8-bit characters, often in documents that have been
converted from Microsoft Office applications.

`epub2txt` does not right-justify text, as there are already many good
utilities to do this (e.g., `groff`)

`epub2txt` extracts text aggressively, and will include things that cannot
possibly be rendered properly in plain text. This includes constructs like
indices and tables of contents, which will be of little use. The captions of
pictures will also likely be included, even though the pictures themselves can
not. It seemed better to err on the side of extracting too much text than too
little; unfortunately there is little in the EPUB format to distinguish content
that is meaningful in a text-only representation from that which is not. 

It is unlikely that any kind of fixed-layout structure of the source document
will be rendered accurately in plain text, so `epub2txt` does not try. Tabs and
other layout elements are collapsed into spaces, and text re-flowed according
to the line length (except in raw mode).

Conversion of Unicode to ASCII is, in the general case, impossible. The
`--ascii` switch tells `epub2txt` to perform some common conversions, such as
straight quotes for angled quotes.  It will also attempt to replace accented
Latin characters with non-accented equivalents, at least for commonly-used
characters. However, there are a huge number of characters in the Unicode set
that cannot be rendered, even approximately, in ASCII. 

Only limited testing has been done with EPUB 3.x documents.

It would be possible to enhance `epub2txt` so that it outputs HTML, or
LaTeX, or PDF; it would be possible to add searching and indexing features;
it would be possible to extend it to include other input formats. 
However, I intended `epub2txt` to be a simple, lightweight utility and,
at present, I consider it to be feature-complete. One day I might develop
a more sophisticated version but, frankly, Calibre already has this
covered.


## Revision history 

Date | Change
-----|-------
2.11,&nbsp;Dec&nbsp;2024 | Added '--separator' option 
2.10,&nbsp;Sep&nbsp;2024 | Rejected links to documents outside the EPUB
2.09,&nbsp;Aug&nbsp;2024 | Improved failure mode wth certain corrupt EPUBs 
2.08,&nbsp;Jun&nbsp;2024 | Fixed a memory-management warning 
?,&nbsp;Jun&nbsp;2024 | Removed position-independent code attributes from defaults 
2.07,&nbsp;Jun&nbsp;2024 | Improved clean-up if program killed in a pipe
?,&nbsp;Jun&nbsp;2022 | Fixed handling of URL-encoded spine href's 
2.06,&nbsp;Jun&nbsp;2022 | Fixed bug in invoking unzip 
2.05,&nbsp;Apr&nbsp;2022 | Fixed bug with empty metadata tags 
2.04,&nbsp;Apr&nbsp;2022 | Improved handling of UTF-8 BOMs 
2.03,&nbsp;Jan&nbsp;2022 | Fixed a buffer overrun bug 
2.02,&nbsp;May&nbsp;2020 | Updated XML parser 
2.01,&nbsp;January&nbsp;2019 | Various bug fixes
2.0,&nbsp;October&nbsp;2017 | Completely re-written to do all text processing using 32-bit character arrays, rather than UTF-8 strings, to improve handling of non-English documents.
0.1.5,&nbsp;September&nbsp;2017 | Some fixes related to line-wrapping with multi-byte characters; support (after a fashion) for manifest files with namespaces.
0.1.4,&nbsp;May&nbsp;2017 | Remove unnecessary KBOX support kludges
0.1.3,&nbsp;March&nbsp;2016 | Fixed a bug that caused epub2txt to fail when XML files contained a UTF-8 BOM
0.1.2,&nbsp;September&nbsp;2015 | Fixed a bug that caused strings like "%222022020," which might legitimately appear in URLs, to be treated as text length specifiers. 
0.1.1,&nbsp;April&nbsp;2015 | Fixed some bugs with integer sizes that caused problems on 64-bit systems
0.0.1 | First functional release

## Author and legal 

`epub2txt` is maintained by Kevin Boone, with contributions from various other
people, and distributed under the terms of the GNU Public Licence (GPL), v3.0.
All the content other than by Kevin Boone is believed to be compatible with,
or less restrictive than, the GPL.

Essentially, this means that you may use this software as you wish, at your
own risk, provided that the original authors continue to be acknowledged.



