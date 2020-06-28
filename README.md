# dinetd - accept connections and run executable.

dinetd binds to any available TCP port, listens, and writes the port
number to stdout (as ASCII digits, followed by a newline).  When a
connenction is accepted, dinetd forks and execs the given executable,
duping the new connection to stdin, stdout, and stderr.

Example usage:

```
   $ dinetd git daemon --inetd --export-all --base-path=/public /public
   12345
   <waits for connections>

   $ git clone git://localhost:12345/my-repo
   <clones /public/my-repo into the current directory>
```
