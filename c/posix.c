/*
 * POSIX half of the command shim: termios raw mode and one-byte reads.
 * isatty and columns live on std::io::stream. A selection prompt has to
 * read arrow keys as CSI, not lines.
 *
 * Raw mode is refcounted. SIGINT/SIGTERM/SIGHUP and atexit restore the
 * original termios so a killed prompt does not leave the tty wedged.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios g_orig;

/*
 * The two modes raw reading alternates between, both derived from g_orig once.
 * g_raw blocks for a byte; g_wait gives up after a tenth of a second, which is
 * how a lone ESC is told from the start of a CSI. Cached because the escape
 * reader flips modes for every byte of a sequence, and asking the driver what
 * it is currently set to - to answer with what we set it to - is two syscalls
 * per byte for an answer we already have.
 */
static struct termios g_raw;
static struct termios g_wait;

static volatile sig_atomic_t g_raw_depth = 0;
static int g_hooks = 0;
static struct sigaction g_old_int;
static struct sigaction g_old_term;
static struct sigaction g_old_hup;

static volatile sig_atomic_t g_cursor_hidden = 0;

/*
 * A live region hides the terminal cursor while a frame is on screen and
 * shows it again itself. This is the path the Echo side cannot take: a
 * signal, or exit. write() is async-signal-safe; the stream the region
 * writes to is not knowable here, so stderr stands in for the terminal.
 */
static void restore_cursor(void)
{
    if (!g_cursor_hidden) {
        return;
    }

    g_cursor_hidden = 0;
    (void)!write(STDERR_FILENO, "\x1b[?25h", 6);
}

/* restore-on-signal is best-effort: tcsetattr is not async-signal-safe. */
static void restore_tty(void)
{
    restore_cursor();

    if (g_raw_depth > 0) {
        (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig);
        g_raw_depth = 0;
    }
}

static void on_signal(int sig)
{
    restore_tty();
    (void)signal(sig, SIG_DFL);
    (void)raise(sig);
}

static void install_hooks(void)
{
    struct sigaction sa;

    if (g_hooks) {
        return;
    }

    (void)atexit(restore_tty);

    sa.sa_handler = on_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    (void)sigaction(SIGINT, &sa, &g_old_int);
    (void)sigaction(SIGTERM, &sa, &g_old_term);
    (void)sigaction(SIGHUP, &sa, &g_old_hup);
    g_hooks = 1;
}

static void uninstall_hooks(void)
{
    if (!g_hooks || g_cursor_hidden) {
        return;
    }

    (void)sigaction(SIGINT, &g_old_int, NULL);
    (void)sigaction(SIGTERM, &g_old_term, NULL);
    (void)sigaction(SIGHUP, &g_old_hup, NULL);
    g_hooks = 0;
}

/*
 * The Echo side flags a hidden cursor here so the hooks above can put it
 * back. Hiding installs them even without raw mode, and the last of the two
 * to finish is the one that takes them down.
 */
int32_t command_cursor_hidden(int32_t on)
{
    if (on) {
        g_cursor_hidden = 1;
        install_hooks();

        return 0;
    }

    g_cursor_hidden = 0;

    if (g_raw_depth <= 0) {
        uninstall_hooks();
    }

    return 0;
}

int32_t command_raw_enter(void)
{
    if (g_raw_depth > 0) {
        g_raw_depth++;
        return 0;
    }

    if (tcgetattr(STDIN_FILENO, &g_orig) != 0) {
        return -1;
    }

    g_raw = g_orig;
    g_raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    g_raw.c_cc[VMIN] = 1;
    g_raw.c_cc[VTIME] = 0;

    g_wait = g_raw;
    g_wait.c_cc[VMIN] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_raw) != 0) {
        return -1;
    }

    g_raw_depth = 1;
    install_hooks();
    return 0;
}

int32_t command_raw_leave(void)
{
    if (g_raw_depth <= 0) {
        return 0;
    }

    if (g_raw_depth > 1) {
        g_raw_depth--;
        return 0;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig) != 0) {
        return -1;
    }

    g_raw_depth = 0;
    uninstall_hooks();
    return 0;
}

int32_t command_read_byte(void)
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n == 1) {
        return (int32_t)c;
    }

    return -1;
}

int32_t command_read_byte_wait(int32_t tenths)
{
    unsigned char c;
    ssize_t n;

    if (g_raw_depth <= 0) {
        return -1;
    }

    if (tenths < 0) {
        tenths = 0;
    }

    if (tenths > 255) {
        tenths = 255;
    }

    g_wait.c_cc[VTIME] = (cc_t)tenths;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &g_wait) != 0) {
        return -1;
    }

    n = read(STDIN_FILENO, &c, 1);
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &g_raw);

    if (n == 1) {
        return (int32_t)c;
    }

    return -1;
}
