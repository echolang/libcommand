---
description: What Spinner, Progress and LogStream share. One trailing row, a tick on another thread, and a pipe that gets plain lines.
---

# Live lines

Work is happening on this thread and you want something on screen that keeps moving. There are three widgets for that:

- A [Spinner](./spinner.md) is a glyph and some words, for work you can't measure.
- A [Progress](./progress.md) bar fills toward a total, for work you can count.
- A [LogStream](./logstream.md) is a scrolling window of output, with a spinner or a bar docked in the same frame.

```echo
$s = $ctx->create<element::Spinner>();
$s->start('fetching');
fetch();
$s->succeed('vendored 4 packages');
```

## One row, from the bottom

`Spinner` and `Progress` share one cursor protocol: a single trailing line, carriage return and erase-to-end-of-line, never cursor-up. Nothing above that row is touched.

A `LogStream` is the exception. A window is more than one row, so it climbs with cursor-up to repaint its frame, the same as a [prompt](./prompts.md).

That's why two live things on one stream erase each other. Each one believes it owns the bottom of the terminal. Dock the bar or the spinner into a `LogStream` and they share one frame instead.

## The tick is the other thread

The animation runs on a second thread, every 80ms. Your thread is free to block on the work, and the glyph keeps moving anyway. `succeed`, `fail`, `finish` and `stop` join that thread before they write the last row, so the finished row is never painted over by a late frame.

Dropping a live widget stops it too.

## Printing while it runs

The live row is the last line of the stream. A diagnostic on that same stream needs `suspend()` first, so the live row is erased before you print.

```echo
$s->suspend();
$ctx->line('cache miss');
```

A `LogStream` doesn't need this: `print` is how a row lands above the window.

## On a pipe

On a pipe nothing rewrites. Painting is silent, and finishing writes one full line. A log file gets the outcome, not 300 frames of spinner.

`examples/05-live.eco` runs a spinner, the `element::spin` shorthand, a bar, and a `suspend()`. `examples/09-logstream.eco` is the window.
