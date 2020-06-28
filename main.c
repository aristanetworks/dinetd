/* Copyright © 2020 Arista Networks, Inc. All rights reserved.
 *
 * Use of this source code is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/ip.h>

#ifndef VERSION
#define VERSION unknown
#endif

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

enum {
	OPTION_VERSION = 128,
	OPTION_ARGV0,
};

/* Usage is generated from usage.txt. Note that the array is not null-terminated,
   I couldn't find a way to convince xxd to do that for me, so instead
   I just replace the last newline with a NUL and print an extra newline
   from the usage function. */
extern unsigned char usage_txt[];
extern unsigned int usage_txt_len;

struct opts {
	char *filename;
	char **argv;
	char **envp;
};

int serve(struct opts *opts);

int version()
{
	puts(STRINGIFY(VERSION));
	return 0;
}

int usage(int error, char *argv0)
{
	usage_txt[usage_txt_len - 1] = 0;
	FILE *out = error ? stderr : stdout;
	fprintf(out, (char *) usage_txt, argv0);
	fprintf(out, "\n");
	return error ? 2 : 0;
}

int main(int argc, char *argv[], char *envp[])
{
	static struct option options[] = {
		{ "help",       no_argument,        NULL,           'h' },

		/* long options without shorthand */
		{ "version",    no_argument,        NULL,           OPTION_VERSION  },
		{ "argv0",      required_argument,  NULL,           OPTION_ARGV0    },

		{ 0, 0, 0, 0 }
	};

	char *argv0 = NULL;
	struct opts opts;
	int error = 0;
	int c;
	while ((c = getopt_long(argc, argv, "+h:", options, NULL)) != -1) {
		switch (c) {
			case 0:
				break;

			case OPTION_VERSION:
				return version(argv[0]);
				break;

			case OPTION_ARGV0:
				argv0 = optarg;
				break;

			case '?':
				error = 1;
				/* fallthrough */
			case 'h':
				return usage(error, argv[0]);
		}
	}

	if (optind + 1 > argc) {
		return usage(1, argv[0]);
	}

	char *new_argv[argc - optind + 1];
	new_argv[0] = argv0 ? argv0 : argv[optind];
	for (int i = 1; i < argc - optind; ++i) {
		new_argv[i] = argv[optind + i];
	}
	new_argv[argc - optind] = NULL;

	opts.filename = argv[optind];
	opts.argv = new_argv;
	opts.envp = envp;
	serve(&opts);
}

void sigchld_handler()
{
	for(;;) {
		int wstatus;
		pid_t pid = wait3(&wstatus, WNOHANG, (struct rusage *) NULL);
		if (pid<=0) {
			return;
		}
	}
}

int serve(struct opts *opts)
{
	int s = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (s<0) {
		err(1, "socket");
	}
	int r = listen(s, 255);
	if (r<0) {
		err(1, "listen");
	}
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	r = getsockname(s, &sin, &len);
	if (r<0) {
		err(1, "getsockname");
	}
	printf("%d\n", ntohs(sin.sin_port));
	fclose(stdout);
	signal(SIGCHLD, sigchld_handler);
	for(;;) {
		len = sizeof(sin);
		int s2 = accept(s, &sin, &len);
		if (s2<0) {
			perror("accept");
			continue;
		}
		int pid = fork();
		if (pid!=0) {
			if (pid<0) {
				perror("fork");
			}
			close(s2);
			continue;
		}
		// Child.
		dup2(s2, 0);
		dup2(s2, 1);
		dup2(s2, 2);
		if (s2>2) {
			close(s2);
		}
		execvpe(opts->filename, opts->argv, opts->envp);
		err(1, "execve");
	}
	return 1; // not reached
}
