// Author: fcwu.tw@gmail.com
// ~ no right reserved ~
//
// fork(2) a child to execute dstat, then use poll(2) and pipe(2) to read 
// it from parent when parent is idle
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <string>
#include <vector>

using std::vector;
using std::string;

int g_exit = false;
vector<string> g_vstr;

void handle_signal(int signo) 
{
    if (SIGCHLD == signo) {
        printf("child dead\n");
        g_exit = true;
    } else {
        printf("signal %d catched\n", signo);
        g_exit = true;
    }
}

static void child(int fd) 
{
    const char * arg_list[] = {"dstat", "--cpu", "--mem", "--net", "--nocolor", NULL};
    dup2(fd, fileno(stdout));
    dup2(fd, fileno(stderr));
    execvp("dstat", (char **)arg_list);
    perror("exevcp");
    exit(EXIT_FAILURE);
}

static void idle()
{
    while (g_vstr.size() > 0) {
        string str = g_vstr[0];
        g_vstr.erase(g_vstr.begin());
        // DORO: Note that the start of str may not the start of output line
        //       and end of str sometimes is not '\n'
        printf("%s", str.c_str());
    }
}

int main() 
{
    /**
     * signal
     */
	struct sigaction act;
	act.sa_handler = handle_signal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);

    /**
     * create child
     */
	int	pipefd[2];
	struct pollfd fds[1];
    pid_t pid;

    if (pipe(pipefd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipefd[0]);
        child(pipefd[1]);
    }
    close(pipefd[1]);
    fds[0].fd = pipefd[ 0 ];
    fds[0].events = POLLRDNORM;

    char buf[128];
    int	nbytes;
    int i;
    while (!g_exit) {
        i = poll(fds, 1, 100);
        if (i == 0) {
            idle();
        } else if (i > 0) {
            if (0 != (fds[0].revents & POLLRDNORM)) {
                nbytes = read(fds[0].fd, buf, sizeof(buf) - 1);
                if (nbytes > 0) {
                    buf[nbytes] = 0;
                    string str(buf);
                    g_vstr.push_back(buf);
                }
            }
            if (0 != (fds[0].revents & POLLHUP)) {
                printf("HUP on fd %d from child %d\n", fds[0].fd, i);
                fds[0].fd = -1;
                g_exit = true;
            }
        } else {
            perror("poll");
            g_exit = true;
        }
    }

    kill(SIGKILL, pid);

    return 0;
}
