---
description: The docker-build window. The last few rows of a stream, with a bar or spinner docked in the same frame.
---

# LogStream

You have a stream of compiler output and a progress bar. If they both rewrite the cursor, they erase each other. If you print every row, the bar scrolls off the top and the terminal fills with noise nobody reads.

[](api:command::element::LogStream) is the docker-build window: only the last few rows, scrolling, and the bar sits in a slot of that same frame.

```echo
$log = $ctx->create<element::LogStream>(4);
$bar = $log->progress();
$bar->start('compile', 8);

foreach ($units as $unit) {
    $log->push("cc -O2 -c src/{$unit}.c -o build/{$unit}.o");
    $log->push("  note: {$unit}.c: 0 warnings");
    $bar->advance(1, "{$unit}.o");
}

$bar->finish();
```

<!-- render: logstream -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  <span style="color:#7d8590">┊ cc -O2 -c src/model.c -o build/model.o</span></span>
<span class="line">  <span style="color:#7d8590">┊   note: model.c: 0 warnings</span></span>
<span class="line">  <span style="color:#7d8590">┊ cc -O2 -c src/codec.c -o build/codec.o</span></span>
<span class="line">  <span style="color:#7d8590">┊   note: codec.c: 0 warnings</span></span>
<span class="line">  <span style="color:#e3b341">⠼</span> compile <span style="color:#e3b341">━━━━━━━━━━━━━━━━━━━━━━━</span><span style="color:#7d8590">━━━━━━━━━━━━━━</span>  62%  codec.o</span></code></pre></div>
<!-- /render -->

The integer is how many rows the window keeps. Older ones scroll out of sight. Streamed rows sit behind a dotted rail and are drawn muted: they're context, not the thing you're watching.

## Docking

`progress()` docks a [bar](./progress.md) under the rows. `spinner()` docks a [spinner](./spinner.md) above them, which is the docker look. One window holds one docked widget, and a second one dies.

The docked widget is an ordinary `Progress` or `Spinner`. You drive it the same way. The only difference is where its row goes.

```echo
$image = $ctx->create<element::LogStream>(4);
$spin = $image->spinner();
$spin->start('building image');
$image->push('Step 2/4 : RUN apk add --no-cache build-base');
$image->print('cached  FROM alpine:3.20');
$image->push('(4/12) Installing gmp (6.3.0-r1)');
```

<!-- render: logstream-spinner -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  cached  FROM alpine:3.20</span>
<span class="line">  <span style="color:#79c0ff">⠴</span> building image</span>
<span class="line">  <span style="color:#7d8590">┊ Step 2/4 : RUN apk add --no-cache build-base</span></span>
<span class="line">  <span style="color:#7d8590">┊ (4/12) Installing gmp (6.3.0-r1)</span></span>
<span class="line">  <span style="color:#7d8590">┊ (5/12) Installing gcc (13.2.1_git20240309-r0)</span></span>
<span class="line">  <span style="color:#7d8590">┊ (6/12) Installing musl-dev (1.2.5-r0)</span></span></code></pre></div>
<!-- /render -->

`print` is a row that stays. It lands above the window and scrolls with the terminal, not with the window. That's where a finished step, a warning or a cached layer goes.

## Closing it

`finish` and `succeed` collapse the window to the docked row. The log was only interesting while it was moving.

`fail` keeps the tail, so you can read what broke.

<!-- render: logstream-failed -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  <span style="color:#7d8590">┊ ld -o build/app build/*.o</span></span>
<span class="line">  <span style="color:#7d8590">┊ ld: undefined symbol 'tensor_free'</span></span>
<span class="line">  <span style="color:#7d8590">┊   referenced from build/trainer.o</span></span>
<span class="line">  <span style="color:#7d8590">┊ error: linker command failed with exit code 1</span></span>
<span class="line">  <span style="color:#ff7b72">■</span> link <span style="color:#ff7b72">━━━━━━━━━━━━━</span><span style="color:#7d8590">━━━━━━━━━━━━━━━━━━━━━━━━━━━</span>  33%  app</span></code></pre></div>
<!-- /render -->

`$collapse` starts true. Set it false and a successful close keeps the window on screen too, docked row included. A window with nothing docked is closed with `close()`, and dropping it closes it as well.

## What push does to the text

`push` streams text into the window. It doesn't need to be one row:

- Newlines split rows. A trailing newline doesn't add an empty one.
- A carriage return keeps only what follows it. `50%\r100%` is `100%`, which is how a child process's own progress line behaves.
- Tabs become four spaces.
- SGR is stripped while `$strip` is on, and it starts on. Colour from a child process was sized for a terminal it no longer owns.

Pushes that land closer together than `$interval` milliseconds (33 by default) are drawn once, on the next tick. A compiler that dumps 400 lines in a burst costs a few paints, not 400. `$interval = 0` paints every push and starts no tick.

## On a pipe

On a pipe the rows stream through as plain lines, rail included, and nothing is rewritten. The docked widget writes its finished row at the end. A CI log gets every line, which is what a CI log is for.

`examples/09-logstream.eco` is a build that succeeds, then one that does not.
