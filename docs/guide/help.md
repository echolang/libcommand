---
description: The help page, one option's prose, and the refusal that mirrors them.
---

# Help

The page is generated from the table. The compact page is one line per option. The paragraphs live one command further in.

```text
greet build --help
greet build --help optimize
```

`greet build --help` is the compact page for that command: usage, the subcommands, then one line per option that command accepts. `greet build --help optimize` is the prose page for one option. The name goes in bare, without its dashes. A leftover word that names no option still draws the compact page. Somebody who typed `greet build --help main.eco` asked for help.

```echo
command::CLIContext $out = .from($parsed, $app, .stdout);
$out->print(command::renderHelp($app, $parsed, $out));
```

[](api:command::renderHelp) reads `$parsed->helpTopicId`. A topic id of zero or more is one option. Below zero is the compact page. You don't pick the page yourself. The parse already did.

## A refusal

A refusal is the same usage block, with the sentence in front of it.

```echo
command::CLIContext $err = .from($r->parsed, $app, .stderr);
$err->print(command::renderError($e, $app, $r->parsed, $err));
```

The sentence is `$e->message()`. `"{$e}"` is that same sentence, which is handy when you are logging it rather than drawing the page. Help goes to stdout. A refusal goes to stderr. Two contexts, because they are two streams.

`--version` prints the version string and a newline, nothing else:

```echo
io::println(command::renderVersion($app));
```

## What the page looks like

On a capable terminal the headings get a left bar and a dim rule, and value trees use the unicode branches from the theme. Colour paints command names, flag spellings and value names, and leaves the summaries alone.

Colour and unicode fail apart. A CI log can have one and not the other. Width 0 means the terminal never said: the help page assumes 80 columns and caps at 100. How that width is chosen is [the context](./context.md).

`examples/03-help.eco` is the greet table, aimed at the compact page, one option, and a refusal.
