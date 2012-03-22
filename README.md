# `libunicorn`

This was created partially out of frustration with the lack of a quality
IRC client library. I guess maybe I'm just too picky...

The goal of `libunicorn` is to provide a flexible API for dealing with
the IRC protocol. While the focus is on clients, `libunicorn` is flexible
enough to be used by servers as well (even TS6 servers where `libunicorn`
has no knowledge of TS6). `libunicorn`'s primary functionality lies in
its parsers and formatters which convert IRC messages (as described by
the grammar in RFC 1459) to and from C strings. In theory, any protocol
which uses IRC-like messages (i.e. ":sender COMMAND arg arg :long arg")
could make use of the parsers and formatters in `libunicorn`.

# Dependencies

`libunicorn` currently depends on `libmowgli-2`. However, the only mowgli
functionalities being used are mowgli's container classes which are a
small portion of the functionality offered by `libmowgli-2`. `libunicorn`
will probably reimplement a few of these to eliminate the `libmowgli-2`
dependency.

The only other depedency is a working standard C library that implements
functions such as `strncat` and `strtok_r`. `libunicorn` aims to avoid
the socket layer and I/O as completely as possible, serving as little
more than a box of tools to be used by larger applications.

# Documentation

This library is not very well-documented, but the source is simple enough
and the functions named sensibly enough that a quick stroll through the
various `.h` and `.c` files should be adequate to your understanding.
