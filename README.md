# libunicorn v0.3

This was created partially out of frustration with the lack of a quality
IRC client library. I guess maybe I'm just too picky...

The goal of libunicorn is to provide a flexible API for dealing with the
IRC protocol. While the focus is on clients, libunicorn is flexible enough
to be used by servers as well (even TS6 servers where libunicorn has no
knowledge of TS6). libunicorn's primary functionality lies in its parsers
and formatters which convert IRC messages (as described by the grammar
in RFC 1459) to and from C strings. In theory, any protocol which uses
IRC-like messages (i.e. ":sender COMMAND arg arg :long arg") could make
use of the parsers and formatters in libunicorn.

This project is licensed under the MIT license, contained in the COPYING
file.

# Support

The official libunicorn channel is irc.staticbox.net #alicorn

# Dependencies

libunicorn currently depends only on libmowgli-2. Although libunicorn only
uses a small subset of the features in libmowgli-2, the library was
created to be used with applications that will make use of libmowgli-2.

# Documentation

This library is not very well-documented, but the source is simple enough
and the functions named sensibly enough that a quick stroll through the
various .h and .c files should be adequate to your understanding.

# Building

    $ aclocal -I m4
    $ autoconf
    $ automake --foreign --add-missing
    [ignore anything that looks like an error here]
    $ ./configure
    $ make
    $ sudo make install

You will also have to manually copy libunicorn.pc to a place that
pkg-config can get at it.
