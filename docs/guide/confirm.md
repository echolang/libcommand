---
description: A yes or no. Arrow keys, or just press y.
---

# Confirm

You are about to overwrite a directory, and you would like the user to have said so first. [](api:command::element::Confirm) is a yes or no, and `ask` returns a `bool`.

```echo
$go = element::confirm($ctx, 'Continue?');
```

<!-- render: confirm -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Continue?</span>
<span class="line">  <span style="color:#79c0ff">● Yes</span> / ○ No</span></code></pre></div>
<!-- /render -->

On a tty, left and right move between the two answers, and enter confirms the lit one. `y` and `n` answer outright, without an enter. The highlighted answer starts on yes. Yes is `true`.

## Starting on no

A destructive question should start on the safe answer. The constructor takes the starting value as its second argument:

```echo
$q = $ctx->create<element::Confirm>('Delete ./build?', false);
bool $wipe = $q->ask();
```

A user who hits enter without reading gets `false`. That's the point.

## Other words

`words` renames the two answers when yes and no are the wrong pair for the question.

```echo
$q = $ctx->create<element::Confirm>('Overwrite ./my-app?');
$q->words('overwrite', 'keep');
bool $wipe = $q->ask();
```

<!-- render: confirm-words -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Overwrite ./my-app?</span>
<span class="line">  ○ overwrite / <span style="color:#79c0ff">● keep</span></span>
<span class="line">  Overwrite ./my-app?  <span style="opacity:.55">keep</span></span></code></pre></div>
<!-- /render -->

That's the question after one right arrow, then the row it collapses to after enter. The first word is still `true`. `words` changes what is drawn, not what is returned.

## On a pipe

The call prints the question, with the current answer in brackets, and reads a line. Anything starting with `y` is yes, anything starting with `n` is no, either case. Anything else, an empty line included, keeps the starting answer.

```bash
printf 'y\n' | echoc run -m . examples/06-prompts.eco
```

`examples/06-prompts.eco` ends with this, after a [select](./select.md) and a [text](./text.md).
