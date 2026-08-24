/*
 * Windows half of the command shim: console raw mode and one-byte reads.
 * isatty and columns live on std::io::stream.
 *
 * Raw mode is refcounted. Ctrl-C / close / break restore the original
 * console mode so a killed prompt does not leave the tty wedged. The
 * handler returns FALSE so the default handler still terminates; there
 * is no cancelled state to hand back.
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <io.h>

#include <stdint.h>
#include <stdlib.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static DWORD g_orig_mode;
static UINT g_orig_cp;
static int g_have_orig = 0;
static volatile LONG g_raw_depth = 0;
static int g_hooks = 0;
static volatile LONG g_cursor_hidden = 0;
static DWORD g_orig_out_mode;
static DWORD g_orig_err_mode;
static int g_have_out_mode = 0;
static int g_have_err_mode = 0;
static int g_vt_on = 0;

static HANDLE stdin_handle(void)
{
    intptr_t raw = _get_osfhandle(0);

    if (raw == -1 || raw == (intptr_t)INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    return (HANDLE)raw;
}

static HANDLE stdout_handle(void)
{
    intptr_t raw = _get_osfhandle(1);

    if (raw == -1 || raw == (intptr_t)INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    return (HANDLE)raw;
}

static HANDLE stderr_handle(void)
{
    intptr_t raw = _get_osfhandle(2);

    if (raw == -1 || raw == (intptr_t)INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    return (HANDLE)raw;
}

static void enable_vt(HANDLE h, DWORD *saved, int *have)
{
    DWORD mode = 0;

    if (h == INVALID_HANDLE_VALUE) {
        return;
    }

    if (!GetConsoleMode(h, &mode)) {
        return;
    }

    if (!*have) {
        *saved = mode;
        *have = 1;
    }

    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static void restore_vt(HANDLE h, DWORD saved, int have)
{
    if (!have || h == INVALID_HANDLE_VALUE) {
        return;
    }

    SetConsoleMode(h, saved);
}

static void borrow_out(void)
{
    if (g_vt_on) {
        return;
    }

    enable_vt(stdout_handle(), &g_orig_out_mode, &g_have_out_mode);
    enable_vt(stderr_handle(), &g_orig_err_mode, &g_have_err_mode);
    g_vt_on = 1;
}

static void unborrow_out(void)
{
    if (!g_vt_on) {
        return;
    }

    if (g_raw_depth > 0 || g_cursor_hidden) {
        return;
    }

    restore_vt(stdout_handle(), g_orig_out_mode, g_have_out_mode);
    restore_vt(stderr_handle(), g_orig_err_mode, g_have_err_mode);
    g_have_out_mode = 0;
    g_have_err_mode = 0;
    g_vt_on = 0;
}

static void restore_cursor(void)
{
    HANDLE err;
    wchar_t seq[] = { 0x1b, L'[', L'?', L'2', L'5', L'h' };
    DWORD written = 0;

    if (InterlockedCompareExchange(&g_cursor_hidden, 0, 1) == 0) {
        return;
    }

    err = stderr_handle();
    if (err == INVALID_HANDLE_VALUE) {
        return;
    }

    WriteConsoleW(err, seq, 6, &written, NULL);
}

static void restore_tty(void)
{
    HANDLE in;

    restore_cursor();

    if (g_raw_depth > 0 && g_have_orig) {
        in = stdin_handle();
        if (in != INVALID_HANDLE_VALUE) {
            SetConsoleMode(in, g_orig_mode);
            SetConsoleCP(g_orig_cp);
        }
    }

    g_raw_depth = 0;
    unborrow_out();
}

static BOOL WINAPI on_ctrl(DWORD type)
{
    if (type != CTRL_C_EVENT && type != CTRL_BREAK_EVENT && type != CTRL_CLOSE_EVENT) {
        return FALSE;
    }

    restore_tty();
    return FALSE;
}

static void install_hooks(void)
{
    if (g_hooks) {
        return;
    }

    (void)atexit(restore_tty);
    SetConsoleCtrlHandler(on_ctrl, TRUE);
    g_hooks = 1;
}

static void uninstall_hooks(void)
{
    if (!g_hooks || g_cursor_hidden) {
        return;
    }

    SetConsoleCtrlHandler(on_ctrl, FALSE);
    g_hooks = 0;
}

int32_t command_cursor_hidden(int32_t on)
{
    if (on) {
        InterlockedExchange(&g_cursor_hidden, 1);
        borrow_out();
        install_hooks();
        return 0;
    }

    InterlockedExchange(&g_cursor_hidden, 0);

    if (g_raw_depth <= 0) {
        uninstall_hooks();
    }

    unborrow_out();
    return 0;
}

int32_t command_raw_enter(void)
{
    HANDLE in;
    DWORD mode = 0;
    DWORD next;

    if (g_raw_depth > 0) {
        g_raw_depth++;
        return 0;
    }

    in = stdin_handle();
    if (in == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (!GetConsoleMode(in, &mode)) {
        return -1;
    }

    g_orig_mode = mode;
    g_orig_cp = GetConsoleCP();
    g_have_orig = 1;

    next = mode;
    next &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    next |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    next |= ENABLE_EXTENDED_FLAGS;
    next &= ~ENABLE_QUICK_EDIT_MODE;

    if (!SetConsoleMode(in, next)) {
        return -1;
    }

    SetConsoleCP(65001);

    g_raw_depth = 1;
    borrow_out();
    install_hooks();
    return 0;
}

int32_t command_raw_leave(void)
{
    HANDLE in;

    if (g_raw_depth <= 0) {
        return 0;
    }

    if (g_raw_depth > 1) {
        g_raw_depth--;
        return 0;
    }

    in = stdin_handle();
    if (in == INVALID_HANDLE_VALUE || !g_have_orig) {
        g_raw_depth = 0;
        unborrow_out();
        return -1;
    }

    if (!SetConsoleMode(in, g_orig_mode)) {
        return -1;
    }

    SetConsoleCP(g_orig_cp);
    g_raw_depth = 0;
    uninstall_hooks();
    unborrow_out();
    return 0;
}

int32_t command_read_byte(void)
{
    HANDLE in = stdin_handle();
    unsigned char c = 0;
    DWORD got = 0;

    if (in == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (!ReadFile(in, &c, 1, &got, NULL) || got != 1) {
        return -1;
    }

    return (int32_t)c;
}

int32_t command_read_byte_wait(int32_t tenths)
{
    HANDLE in;
    unsigned char c = 0;
    DWORD got = 0;
    DWORD ms;

    if (g_raw_depth <= 0) {
        return -1;
    }

    in = stdin_handle();
    if (in == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (tenths < 0) {
        tenths = 0;
    }

    if (tenths > 255) {
        tenths = 255;
    }

    ms = (DWORD)tenths * 100;

    if (WaitForSingleObject(in, ms) != WAIT_OBJECT_0) {
        return -1;
    }

    if (!ReadFile(in, &c, 1, &got, NULL) || got != 1) {
        return -1;
    }

    return (int32_t)c;
}
