---
description: Rows of strings under a header, sized to the widest cell.
---

# Table

You have rows of strings and you want them in columns, each one as wide as its widest cell. [](api:command::element::Table) does that and nothing else. There is no sorting and no wrapping, and a cell is a string.

```echo
$t = $ctx->create<element::Table>();
$t->columns(['Package', 'Version', 'Status']);
$t->row(['libcommand', '0.1.3', 'ready']);
$t->row(['libjson', '0.4.0', 'ready']);
$t->row(['libhttp', '0.2.1']);
$t->display();
```

<!-- render: table -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">┌────────────┬─────────┬────────┐</span>
<span class="line">│ <span style="font-weight:700">Package</span>    │ <span style="font-weight:700">Version</span> │ <span style="font-weight:700">Status</span> │</span>
<span class="line">├────────────┼─────────┼────────┤</span>
<span class="line">│ libcommand │ 0.1.3   │ ready  │</span>
<span class="line">│ libjson    │ 0.4.0   │ ready  │</span>
<span class="line">│ libhttp    │ 0.2.1   │        │</span>
<span class="line">└────────────┴─────────┴────────┘</span></code></pre></div>
<!-- /render -->

The header is bold when the context has colour. That's the only colour a table owns. Anything else in a cell is yours, so a `command::styled` word stays styled, and the width math skips its SGR.

## Columns, then rows

`columns` takes the headers in one array. `column` adds one. Do either before the rows: a row is checked against the columns that exist when you push it.

Here is the catch: `row()` dies if the row is longer than the columns. A short row is fine, which is what `libhttp` above is doing. The missing cells stay empty.

## The border and the header

`$border` and `$showHeader` are public fields, not setters. Both start on.

```echo
$t->border = false;
$t->display();
```

<!-- render: table-borderless -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line"> <span style="font-weight:700">Package</span>     <span style="font-weight:700">Version</span>  <span style="font-weight:700">Status</span> </span>
<span class="line"> libcommand  0.1.3    ready  </span>
<span class="line"> libjson     0.4.0    ready  </span>
<span class="line"> libhttp     0.2.1           </span></code></pre></div>
<!-- /render -->

Without a border, the columns are still padded and still one space apart on each side. `$showHeader = false` drops the header row, and with it the rule under the header.

The border glyphs come from the unicode theme when the context has unicode, and from `+`, `-`, `|` when it does not. Colour is not part of that choice. [Widgets](./widgets.md) has the ascii version.

## display and render

`display()` writes the table through the context it was minted with. `render()` hands back the same page as a [](api:command::Block), for when you are composing it into something bigger.

In a `buffer` context, `display()` writes nothing. The rows land on `$t->captured`, which is how a test reads them back.

`examples/04-widgets.eco` draws a table, then a [tree](./tree.md).
