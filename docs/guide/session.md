---
description: beginStepper opens one connected run. The widgets are the same widgets.
---

# A session

A wizard is one connected run. `beginStepper` / `endStepper` on the context opens it. Widgets look up the live channel when they draw, so a widget minted before the run still hangs off the bar once you open one.

```echo
$ctx->beginStepper('create-app');

$dir = element::text($ctx, 'Where should we create it?', './my-app');
$type = element::select($ctx, 'Pick a project type', $types);
$go = element::confirm($ctx, 'Continue?');

$spin = $ctx->create<element::Spinner>();
$spin->start('writing files');
$spin->succeed('wrote 12 files');

$ctx->endStepper('Done');
```

<!-- render: session -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line"><span style="color:#7d8590">┌</span>  create-app</span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7ee787">◇</span>  Where should we create it?</span>
<span class="line"><span style="color:#7d8590">│</span>  <span style="opacity:.55">./my-app</span></span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#56d4dd">◆</span>  Pick a project type</span>
<span class="line"><span style="color:#56d4dd">│</span>  <span style="color:#7ee787">● </span>TypeScript</span>
<span class="line"><span style="color:#56d4dd">│</span>  <span style="opacity:.55">○ JavaScript</span></span>
<span class="line"><span style="color:#56d4dd">│</span>  <span style="opacity:.55">○ Echo</span></span>
<span class="line"><span style="color:#56d4dd">│</span>  <span style="opacity:.55">↑/↓</span> to navigate • <span style="opacity:.55">Enter:</span> confirm</span>
<span class="line"><span style="color:#56d4dd">└</span></span></code></pre></div>
<!-- /render -->

`◆` is the step waiting for you. `◇` is one that answered. The channel is drawn in the colour of the step it belongs to, `└` included, and it settles to grey the moment that step is answered. The run reads as one lit stretch in a column of finished ones.

That's the run halfway, with the type question still open. Here is the same run once everything answered, a [note](#same-widgets) and a spinner included, closed with `endStepper('Done')`:

<!-- render: session-done -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line"><span style="color:#7d8590">┌</span>  create-app</span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7ee787">◇</span>  Where should we create it?</span>
<span class="line"><span style="color:#7d8590">│</span>  <span style="opacity:.55">./my-app</span></span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7ee787">◇</span>  Pick a project type</span>
<span class="line"><span style="color:#7d8590">│</span>  Echo</span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7d8590">◇</span>  Heads up ─────────────────╮</span>
<span class="line"><span style="color:#7d8590">│</span>                            <span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7d8590">│</span>  This overwrites ./my-app  <span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7d8590">│</span>                            <span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7d8590">├</span>────────────────────────────╯</span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7ee787">◇</span>  Continue?</span>
<span class="line"><span style="color:#7d8590">│</span>  <span style="opacity:.55">Yes</span></span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7ee787">✔</span>  wrote 12 files</span>
<span class="line"><span style="color:#7d8590">│</span></span>
<span class="line"><span style="color:#7d8590">└</span>  Done</span></code></pre></div>
<!-- /render -->

Every answer is the title on a `◇` with the value on the bar below it. That's what stays in the scrollback, and it reads as a transcript.

Dropping the context while a run is still open closes it too. `endStepper` is how the last row gets to say something. The no-argument form closes with an empty message. Calling `beginStepper` while one is already open dies.

## Same widgets

[Prompts](./prompts.md), [spinners](./spinner.md) and [progress bars](./progress.md) are still minted from the context. `Spinner` and `Progress` join the bar when they are created during a run: their glyph moves into the bar column. `element::spin` still takes a [](api:command::CLIContext). During a run it still joins the bar, because the spinner is minted from that same context.

A program that wants one prompt doesn't have to open a run to get it. With no stepper open, every widget draws the way it always has, two spaces in.

```echo
command::Block $note = element::noteFrame($ctx->channel(), 'Heads up', 'This overwrites ./app');
$ctx->print($note);
```

`noteFrame` is the boxed aside that hangs off the bar. `printError` during a run is a mark on that same channel. Off a run it is the `error:` badge.

Colour is still a capabilities question, now sitting on the context. Without it the run keeps its shape and loses its highlights. Without unicode the channel falls back to `| + > o`. One column either way, so nothing shifts.

`examples/07-session.eco` is the run. `examples/08-create-app.eco` is argv, then a session, then a summary table.
