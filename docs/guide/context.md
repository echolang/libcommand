---
description: CLIContext is the terminal a program is talking to, named once.
---

# The context

Widgets, help and refusals all need to know what they are drawing on. That pair (what the terminal can do, and which stream) is a [](api:command::CLIContext). Name it once.

```echo
command::CLIContext $ctx = .resolve(.auto, .stderr);
```

`$ctx->caps` is a [](api:command::TerminalCapabilities): colour, unicode, width, and whether the stream is a terminal. `$ctx->out` is the stream. Everything minted from this context writes there.

## Colour, then unicode

`--color=always` and `--color=never` win outright. `auto` asks the environment, in this order:

1. `NO_COLOR` set to anything: no colour.
2. `CLICOLOR_FORCE` set to something other than `0`: colour.
3. `TERM=dumb`: no colour.
4. Otherwise, colour when the stream is a terminal.

Unicode is a different question. It is on when the stream is a terminal and the locale says UTF-8 (`LC_ALL`, `LC_CTYPE`, or `LANG`), or when `WT_SESSION` is set. A pipe gets neither, whatever the locale says.

`COLUMNS` is the width when the stream is a terminal. Width 0 means unknown. Help assumes 80 and caps at 100. A live line assumes 80 and still truncates. A column, for that measurement, is one Unicode scalar, and that scalar has width 1.

```echo
command::CLIContext $ctx = .from($parsed, $app, .stderr);
```

`from` is the same resolution, honouring the `--color` the parse already took. A program that paints help on stdout and widgets on stderr makes two contexts.

`command::styled($text, command::SGR_INFO, $ctx->caps->color)` wraps a word in an SGR sequence when colour is on, and returns the word alone when it is not. The off sequence closes the attribute you opened. It does not reset the whole terminal.

## Tests

Tests mint a context that never touches the host tty.

```echo
command::CLIContext $ctx = .plain();
command::CLIContext $wide = .buffer(command::TerminalCapabilities(true, true, 80, true));
```

`plain` is no colour, no unicode, width 0, not interactive. `buffer` keeps whatever capabilities you hand it, and records finished rows on `$ctx->page` instead of writing them. Raw VT (a spinner repainting its line) lands in `$ctx->vt`.

## Minting a widget

`create<T>` is a constructor call on T, with this context already in its hand. Echo has no variadics, so arity is overloads: `create<Spinner>()`, `create<Text>('What is your name?')`, `create<Select>('Which?', SelectKind::checks)`. At most two extra arguments. Two creates are two objects.

```echo
$t = $ctx->create<element::Table>();
$t->column('Feature');
$t->column('Status');
$t->row(['Help', 'ready']);
$t->display();
```

`$t->display()` writes through the context the widget was minted with. The stream and the colour decision were made once, at `resolve` or `from`.

A session is the same context, with a channel open down the left. That is [a session](./session.md). The things you draw with no channel open start at [widgets](./widgets.md).
