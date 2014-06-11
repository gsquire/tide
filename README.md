tide
====

A multi-threaded HTTP server that only responds to GET requests and works from
the directory it is started in.

Usage:

    tide -p [PORT] -6 -h

Options:

    -p [PORT]: Specifies the port to listen on (default asks OS for a port)
    -6:        Create an IPV6 socket
    -h:        Display the help and exit
