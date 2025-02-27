# macOS supplement to epub2txt

Version 2.10, September 2024 

## What is this? 

This macOS-specific README outlines changes required to compile, install, and run `epub2txt` on computers running macOS. This README assumes familiarity with terminal commands, creating directories and compiling applications as per the main README. No additional software or downloads are required beyond Xcode command-line tools (`Xcode-select --install`) and/or Brew development tools (`brew install devtools`). Please consult the main README for non macOS-specific information. 


## Building and installing 

If you've already tried compiling `epub2txt` on macOS, you'll see a number of errors to the effect of `undeclared library function`. To address these errors, edit the `Makefile` and add a parameter to `EXTRA_CFLAGS` as follows::

`EXTRA_CFLAGS ?= -D_DARWIN_C_SOURCE`

This ensures macOS-specific libraries are used. 

With this change, the `make` command will work, but `sudo make install` requires additional changes to the `Makefile`. If you're planning to always run `epub2txt` from a local directory and do not require a system-wide install, the next step is not required.

If you are planning to run `sudo make install` to install `epub2txt` to run from any directory in the command line for all users, modify the `Makefile` as follows:

```
PREFIX  := usr/local
BINDIR  := bin
MANDIR  := share/man
```

...

```
install:
	mkdir -p $(DESTDIR)/$(PREFIX)/$(BINDIR)
	mkdir -p $(DESTDIR)/$(PREFIX)/$(MANDIR)/man1
	install -m 755 $(APPNAME) $(DESTDIR)/$(PREFIX)/$(BINDIR)/
	install -m 644 man1/epub2txt.1 $(DESTDIR)/$(PREFIX)/$(MANDIR)/man1/
```

Save all `Makefile` changes and run

    $ make
    $ sudo make install

Note that `/usr/local` paths for binaries and man pages are by default in the system path. At this point, you should be able to run:

    $ epub2txt
    $ man epub2txt

Note that running `epub2txt` without parameters will simply indicate: `no files selected`.

## Running

If you try to run the application with a valid epub file as a parameter, e.g.:

    $ epub2txt PrincessOfMars-EPUB2.epub

You will likely receive an error to the effect of `Bad OPF rootfile path`

This is because on macOS, the default temporary directory is a symbolic link. `/tmp` actually points to `/private/tmp`, which causes `epub2txt` to think that the epub structure is invalid. 

Two potential solutions:

1. Set a temporary directory environment variable on the same line as executing the command, ensuring it doesn't point to `/tmp` e.g.,

`TMP=/Volumes/NEXT/temp epub2txt PrincessOfMars-EPUB2.epub`

This will set the `TMP` variable while the application is in scope and automatically remove it when the program exits. Note that `/Volumes/NEXT/temp` is provided as an example; the full path passed for `TMP` needs to be created specific to a machine before running the application.

OR

2. Update a section of code to run the `tempbase` directory variable through `realpath` to ensure the temporary directory `/tmp` is expanded to `/private/tmp`, if necessary. Edit `epub2txt.c` and apply the following change:

Before the line: 

`log_debug ("tempbase is: %s", tempbase);`

Add: 

`tempbase = realpath (tempbase, NULL);`

This will add `/private` to the `tempbase` path (if required/encountered) and allow the `is_subpath` validity check to work as intended.

After saving the change, run 

    $ make
    $ sudo make install

With this solution, there's no need to set or pass a `TMP` directory. The system's underlying `TMP` directory will work invisibly.
