---
description: Spinner, Progress, and the LogStream window they can sit in.
---

# Live lines

Work is happening on this thread and you want a line that keeps moving. [](api:command::element::Spinner) and [](api:command::element::Progress) share one cursor protocol: a single trailing line, carriage return and erase-to-end-of-line, never cursor-up. The animation is the other thread, so the glyph keeps moving until you `succeed`, `fail`, or `stop`.

```echo
$s = $ctx->create<element::Spinner>();
$s->start('fetching');
fetch();
$s->update('unpacking');
unpack();
$s->succeed('vendored 4 packages');
```

`start` paints the first frame and arms the tick. `update` changes the words. The glyph keeps cycling. `succeed` and `fail` join that thread, then write one finished row. `stop` erases the line and leaves nothing. Dropping the spinner also stops it.

On a pipe they do not rewrite. `paint` is silent, and `commit` / `succeed` write a full line. A diagnostic on the same stream needs `suspend()` first, so the live row is erased before you print.

```echo
$s->suspend();
$ctx->line('cache miss');
```

## The short spelling

`element::spin` takes a context, a label, and the work. There is no other overload. The value the closure returns is the value `spin` returns, and the finished row uses the same label you started with.

```echo
$body = element::spin($ctx, 'fetching', function() : string {
    return fetch();
});
```

## A bar

`Progress` is a cooperative bar. You `start` with a total, `advance` as work lands, `finish`. The fill moves only when you say something changed. A spinner on the row keeps moving until you commit.

```echo
$bar = $ctx->create<element::Progress>();
$bar->start('compile', 42);
$bar->advance(1, 'model.o');
$bar->set(40, 'link');
$bar->finish();
```

`advance` adds. `set` is an absolute count. Both take a detail string, and an empty string is a fine detail. `finish` fills the bar and settles the row. `fail` settles it as a failure, at whatever count you had reached. `suspend` is the same idea as on a spinner: erase the live row so something else can use the stream.

`examples/05-live.eco` runs a spinner, the shorthand, a bar, and a `suspend()`.

## A window of lines

You have a stream of compiler output and a progress bar. If they both rewrite the cursor, they erase each other. [](api:command::element::LogStream) is the docker-build window: only the last few rows, scrolling, and the bar (or spinner) sits in a slot of that same frame.

```echo
$log = $ctx->create<element::LogStream>(6);
$bar = $log->progress();
$bar->start('compile', 42);
$log->push('cc -O2 -c src/model.c');
$bar->advance(1, 'model.o');
$bar->finish();
```

The integer is how many rows the window keeps. `progress()` docks a bar under the rows. `spinner()` docks a spinner above them. `finish` and `succeed` collapse the window to that docked row. `fail` keeps the tail, so you can read what broke.

`push` streams text into the window. Newlines split rows. A carriage return keeps only what follows it, which is how a child process's own progress line behaves. On a pipe the rows stream through as plain lines, and nothing is rewritten.

`$log->print('cached')` is a row that stays. It lands above the window and scrolls with the terminal, not with the window.

`$collapse` starts true. Set it false and `close` keeps the window on screen, docked row included.

`examples/09-logstream.eco` is a build that succeeds, then one that does not.
