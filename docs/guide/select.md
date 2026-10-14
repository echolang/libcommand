---
description: A list to pick from. One item, or many. Arrow keys on a tty, a numbered list on a pipe.
---

# Select

You have a fixed set of answers and you want the user to pick one, or several. Typing `TypeScript` correctly is not the user's job. [](api:command::element::Select) is a list with a cursor.

```echo
$fruit = element::select($ctx, 'Pick a fruit', ['Apple', 'Banana', 'Orange']);
```

<!-- render: select-radio -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Pick a fruit</span>
<span class="line">    ○ Apple</span>
<span class="line">  <span style="color:#79c0ff">❯ ● Banana</span></span>
<span class="line">    ○ Orange</span>
<span class="line">  <span style="opacity:.55">↑/↓</span> to navigate • <span style="opacity:.55">Enter:</span> confirm</span></code></pre></div>
<!-- /render -->

That's the list one key in, with the cursor moved down to `Banana`. Up and down move, enter confirms. The focused row carries the pointer and the colour, and the filled radio follows the cursor.

`ask` on a Select always returns `array<string>`. Radio is length 1, so the fruit above is `$fruit[0]`. One return type for both kinds means one code path in the caller, and I'd rather index a one-element array than branch on what the menu was.

## Checks

`SelectKind::checks` is many answers. Space toggles the row under the cursor, enter confirms the lot.

```echo
$q = $ctx->create<element::Select>('Which features?', element::SelectKind::checks);
$q->choices(['Logging', 'Caching', 'Compression', 'Metrics']);
$q->pick('Logging');
$on = $q->ask();
```

<!-- render: select-checks -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Which features?</span>
<span class="line">    ◼ Logging</span>
<span class="line">    ◻ Caching</span>
<span class="line">    ◼ Compression</span>
<span class="line">  <span style="color:#79c0ff">❯ ◻ Metrics</span></span>
<span class="line">  <span style="opacity:.55">↑/↓</span> to navigate • <span style="opacity:.55">Space:</span> select • <span style="opacity:.55">Enter:</span> confirm</span></code></pre></div>
<!-- /render -->

`pick` marks `Logging` before the menu opens. On radio, `pick` moves the cursor there instead. Either way it is also what an empty line means on a pipe.

`choice` adds one label, `choices` adds the array. `element::select` is the short spelling: title, choices, and radio unless you pass `SelectKind::checks` as the fourth argument.

## After enter

The menu collapses to one row: the title, then what was chosen.

<!-- render: select-done -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Which features?  Logging, Compression</span></code></pre></div>
<!-- /render -->

That row is what stays in the scrollback. Inside a [session](./session.md) the answer gets its own row under the title instead.

## Knobs

- `pageSize(n)` is how many rows show before the list scrolls. It starts at 10.
- `wrap(true)` lets the cursor leave the last item and land on the first. It starts off.
- `require(false)` is a checks menu that may confirm with nothing marked. `require` starts on, and enter on an empty menu just does nothing.

## On a pipe

On a pipe there are no keys to read. The same `ask()` prints a numbered list and reads one line.

<!-- render: select-pipe -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Pick a fruit</span>
<span class="line">  1) Apple</span>
<span class="line">  2) Banana</span>
<span class="line">  3) Orange</span></code></pre></div>
<!-- /render -->

`2` picks radio item 2. `1,3` or `1 3` picks checks. An empty line uses `pick` if you set one, otherwise it's a refusal. So does a number the list doesn't have.

```bash
printf '2\n' | echoc run -m . examples/06-prompts.eco
```

`tryAnswer` is the same parse without the refusal. It returns null for a line the list doesn't accept, which is how you'd test a menu without a terminal.

`examples/06-prompts.eco` is a radio, then checks, then a [text](./text.md) and a [confirm](./confirm.md).
