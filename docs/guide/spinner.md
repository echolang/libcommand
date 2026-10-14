---
description: A glyph that keeps moving while this thread does the work.
---

# Spinner

Work is happening on this thread and you don't know how long it takes. You want something on screen that says "still alive". [](api:command::element::Spinner) is one line: a glyph that keeps cycling, and the words next to it.

```echo
$s = $ctx->create<element::Spinner>();
$s->start('fetching echolang/libjson');
fetch();
```

<!-- render: spinner -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  <span style="color:#79c0ff">⠹</span> fetching echolang/libjson</span></code></pre></div>
<!-- /render -->

The animation is the other thread. `start` paints the first frame and starts a tick every 80ms. Your thread is free to block, and the glyph keeps moving anyway.

## Finishing it

`succeed` and `fail` join that thread, then write one finished row. The glyph becomes a mark, and the words become whatever you pass.

```echo
$s->succeed('vendored 4 packages');
$s->fail('mirror refused the connection');
```

<!-- render: spinner-done -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  <span style="color:#7ee787">✔</span> vendored 4 packages</span>
<span class="line">  <span style="color:#ff7b72">■</span> mirror refused the connection</span></code></pre></div>
<!-- /render -->

That's two spinners, one of each. A spinner finishes once.

`stop` joins the thread, erases the line, and leaves nothing behind. Dropping the spinner also stops it, so a spinner that goes out of scope mid-run doesn't keep a thread spinning on a line nobody owns.

`update` changes the words while it runs. The glyph keeps cycling.

```echo
$s->start('fetching');
fetch();
$s->update('unpacking');
unpack();
$s->succeed('vendored 4 packages');
```

## Printing while it spins

The spinner owns the last line of the stream. Print something on that same stream while it's live, and the next frame paints over it.

`suspend()` pauses the tick and erases the row. Print your line. Here is the catch: nothing un-pauses a spinner. `update` changes the words but the row stays off, so a suspended spinner is something you finish next, with `succeed`, `fail` or `stop`. (A [bar](./progress.md) is different: its next `advance` paints again.)

```echo
$s->suspend();
$ctx->line('mirror is slow, using the fallback');
$s->succeed('vendored 4 packages');
```

## The short spelling

`element::spin` takes a context, a label, and the work. The value the closure returns is the value `spin` returns, and the finished row uses the same label you started with.

```echo
$body = element::spin($ctx, 'fetching', function() : string {
    return fetch();
});
```

There is no other overload. When the finished row should say something different from the running one, mint a `Spinner`.

## On a pipe

A pipe doesn't get frames. `start` paints nothing, and `succeed` or `fail` write one full line. A CI log ends up with the result and none of the animation.

Without unicode the glyph cycles `|`, `/`, `-`, `\`, and the marks are `*` and `x`.

`examples/05-live.eco` runs a spinner, a `suspend()`, the short spelling, then a [bar](./progress.md).
