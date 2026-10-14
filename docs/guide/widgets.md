---
description: How a widget is minted, how it leaves, and why the glyphs are a unicode question.
---

# Widgets

A program that only wants a table never names `Command`. What it does name is a [](api:command::CLIContext), the terminal those widgets will draw on. Widgets live in `command::element`.

```echo
use command;
use command::element;

command::CLIContext $ctx = .resolve(.auto, .stderr);

$t = $ctx->create<element::Table>();
$t->column('Feature');
$t->column('Status');
$t->row(['Help', 'ready']);
$t->display();
```

`create<T>` is a constructor call on T, with the context already in its hand. `$t->display()` writes through that context, so the stream and the colour decision were made once, at `resolve` or `from`.

There are three kinds, and each has its own page:

- **Pages you draw once.** A [Table](./table.md) and a [Tree](./tree.md). Build it, `display()` it, done.
- **Live lines.** A [Spinner](./spinner.md), a [Progress](./progress.md) bar, and the [LogStream](./logstream.md) window they can sit in. They keep repainting until you finish them. [Live lines](./live.md) is what they share.
- **Prompts.** A [Select](./select.md), a [Text](./text.md), a [Confirm](./confirm.md). They draw, read keys, and return an answer. [Prompts](./prompts.md) is what they share.

## display and render

`Table` and `Tree` also have `render()`, which returns the page as a [](api:command::Block) if you are composing. `display()` is how a widget leaves.

In a `buffer` context, `display` records the block on the widget's `$captured` instead of writing it. That's how the tests read a table back without a terminal.

## Glyphs are a unicode question

Colour and unicode fail apart. A CI log can render SGR and still choke on box drawing, and a golden test wants the unicode layout with colour off. So the glyphs come from the unicode setting, and colour is decided separately.

Here is the [table](./table.md) on a terminal with colour and no unicode:

<!-- render: table-ascii -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">+------------+---------+--------+</span>
<span class="line">| <span style="font-weight:700">Package</span>    | <span style="font-weight:700">Version</span> | <span style="font-weight:700">Status</span> |</span>
<span class="line">+------------+---------+--------+</span>
<span class="line">| libcommand | 0.1.3   | ready  |</span>
<span class="line">| libjson    | 0.4.0   | ready  |</span>
<span class="line">| libhttp    | 0.2.1   |        |</span>
<span class="line">+------------+---------+--------+</span></code></pre></div>
<!-- /render -->

Same columns, same bold header, `+`, `-` and `|` for the border. Every widget has an ascii set, and each widget page says what it is.

`examples/04-widgets.eco` draws a table and a tree.
