---
description: Table and Tree. A program that only wants one of them never names Command.
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

`create<T>` is a constructor call on T, with the context already in its hand. `$t->display()` writes through that context, so the stream and the colour decision were made once at `resolve` or `from`.

`Table` and `Tree` also have `render()`, which returns the page as a [](api:command::Block) if you are composing. `display()` is how a widget leaves. In a `buffer` context, `display` records the block instead of writing it.

`examples/04-widgets.eco` draws both.

## A table

`$showHeader` and `$border` are public fields, not setters. Both start on. `columns` takes the headers in one array. `column` adds one.

```echo
$t->columns(['Feature', 'Status']);
$t->row(['Help', 'ready']);
$t->showHeader = true;
$t->border = false;
```

Here is the catch: `row()` dies if the row is longer than the columns. A short row is fine. The missing cells stay empty.

The border glyphs come from the unicode theme when the context has unicode, and from `+`, `-`, `|` when it does not. Colour is not part of that choice.

## A tree

A `Tree` is labels, and you build it by pushing into `$node->children[]`.

```echo
$tree = $ctx->create<element::Tree>('root');
element::Node $src = element::Node('src');
$src->children[] = element::Node('main.eco');
$tree->root->children[] = $src;
$tree->display();
```

The constructor argument is the root label. `render` walks `$root` and the children you pushed. The branch glyphs are `├─` and `└─` when unicode is on. Without unicode they are `|-` and a backtick followed by `-`. Same rule as the table: the theme is a unicode question, and colour is a different one.
