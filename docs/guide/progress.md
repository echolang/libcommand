---
description: A cooperative bar. It fills when you say something changed.
---

# Progress

You know how much work there is: 42 files to compile, 8 archives to download. A [spinner](./spinner.md) would throw that away. [](api:command::element::Progress) is a bar that fills toward a total.

```echo
$bar = $ctx->create<element::Progress>();
$bar->start('compile', 42);

foreach ($units as $unit) {
    compile($unit);
    $bar->advance(1, "{$unit}.o");
}

$bar->finish();
```

<!-- render: progress -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  <span style="color:#e3b341">⠼</span> compile <span style="color:#e3b341">━━━━━━━━━━━━━━━━━━━━━━━</span><span style="color:#7d8590">━━━━━━━━━━━━━━</span>  64%  model.o</span></code></pre></div>
<!-- /render -->

The bar is cooperative. The fill moves only when you `advance` or `set`. The glyph on the left keeps moving on its own, so a long file still looks alive.

## advance and set

`advance` adds to the count. `set` is an absolute count. Both take a detail string, the bit after the percent. An empty string is a fine detail.

```echo
$bar->advance(1, 'model.o');
$bar->set(40, 'link');
```

A count past the total is clamped to the total, and a negative one to zero. The bar never claims 120%.

## Finishing it

`finish` fills the bar and settles the row. `fail` settles it as a failure, at whatever count you had reached.

<!-- render: progress-done -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  <span style="color:#7ee787">✔</span> compile <span style="color:#7ee787">━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━</span> <span style="color:#7ee787">100%</span></span>
<span class="line">  <span style="color:#ff7b72">■</span> download <span style="color:#ff7b72">━━━━━━━━━━━</span><span style="color:#7d8590">━━━━━━━━━━━━━━━━━━━━</span>  37%  codec.tar.gz</span></code></pre></div>
<!-- /render -->

The glyph and the fill are one colour on purpose: they're the same state saying the same thing. Amber while it runs, green once it's finished, red when it failed. The percent is the one exception. It turns green at 100% whatever the outcome, because 100% is a count, not a verdict. The glyph and the fill are what say whether the run made it.

## Width

The bar takes whatever the row has left after the label, the percent and the detail, and stops at 40 cells. A long detail makes a shorter bar. The row never wraps, because a wrapped row is a row the next repaint doesn't know about.

## On a pipe

No repaint, no frames. `start`, `advance` and `set` are silent. `finish` and `fail` write the finished row once. Without colour the empty part of the track is spaces, and without unicode the fill is `-`.

`suspend` is the same idea as on a spinner: erase the live row, print your line, and the next `advance` paints the bar again.

## Next to streamed output

A bar and a scrolling log on one stream erase each other. Dock the bar under a [LogStream](./logstream.md) instead, and the two share one frame.

`examples/05-live.eco` runs a spinner, then a bar.
