bst(1) "dinetd" "Dynamic Internet Daemon"

# NAME

dinetd - accept connections and run executable.

# SYNOPSIS

dinetd [options] [--] <executable> [args...]

# DESCRIPTION

dinetd binds to any available TCP port, listens, and writes the port
number to stdout (as ASCII digits, followed by a newline).  When a
connenction is accepted, dinetd forks and execs the given executable,
duping the new connection to stdin, stdout, and stderr.

# OPTIONS

\--argv0 <string>
	Run *<executable>* with the specified value for _argv[0]_.
	By default, _argv[0]_ is *<executable>* itself.

\--version
	Display version information and exit.

# SEE ALSO

*inetd*(8)
