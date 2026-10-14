---
description: Labels and the labels under them, drawn with branch glyphs.
---

# Tree

A directory listing, a dependency graph you have already flattened into a tree, a module and its parts. [](api:command::element::Tree) is labels, and you build it by pushing into `$node->children[]`.

```echo
$tree = $ctx->create<element::Tree>('libcommand');

element::Node $el = element::Node('element');
$el->children[] = element::Node('table.eco');
$el->children[] = element::Node('tree.eco');

element::Node $src = element::Node('src');
$src->children[] = $el;
$src->children[] = element::Node('parse.eco');

$tree->root->children[] = $src;
$tree->root->children[] = element::Node('module.eco');
$tree->display();
```

<!-- render: tree -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">libcommand</span>
<span class="line">├─ src</span>
<span class="line">│ ├─ element</span>
<span class="line">│ │ ├─ table.eco</span>
<span class="line">│ │ └─ tree.eco</span>
<span class="line">│ └─ parse.eco</span>
<span class="line">└─ module.eco</span></code></pre></div>
<!-- /render -->

The constructor argument is the root label. `$tree->root` is a [](api:command::element::Node), the same as every node under it, and `render` walks it depth first in the order you pushed.

## Build it bottom up

A `Node` is a struct, and structs copy on assignment. `$src->children[] = $el` pushes a copy of `$el` as it is right now. Push into `$el` after that, and the tree never sees it.

So build the leaves first, then hand each finished node to its parent. That's why `element` is complete before `src` takes it in the example above.

## Without unicode

The branch glyphs are `├─` and `└─` when unicode is on. Without unicode they are `|-` and a backtick followed by `-`.

<!-- render: tree-ascii -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">libcommand</span>
<span class="line">|- src</span>
<span class="line">| |- element</span>
<span class="line">| | |- table.eco</span>
<span class="line">| | `- tree.eco</span>
<span class="line">| `- parse.eco</span>
<span class="line">`- module.eco</span></code></pre></div>
<!-- /render -->

Same rule as the [table](./table.md): the theme is a unicode question, and colour is a different one. A tree has no colour of its own either way.

## display and render

`display()` writes through the context. `render()` returns the page as a [](api:command::Block). In a `buffer` context `display()` lands on `$tree->captured` instead of writing.

`examples/04-widgets.eco` draws a table, then this.
