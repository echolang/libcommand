---
layout: home
hero:
  name: libcommand
  text: A CLI library for Echo
  tagline: argv becomes values. The help page is that same table. Then you draw something.
  actions:
    - theme: brand
      text: Get started
      link: /guide/
features:
  - title: One table
    details: Parse, usage, the help page and every refusal read the same rows. A handle is an index, so a lookup cannot miss a name.
  - title: The page is the table
    details: One line per option. The paragraphs live one command further in, on that option's own help.
  - title: A few things to draw
    details: A table, a tree, a spinner, a bar, a window of log lines. They all draw on the context you named once.
  - title: Three prompts
    details: A list, a typed line, a yes or no. On a pipe the same call reads a line of stdin.
---

You are writing a command-line program. You need argv to become values, a `--help` page that stays true to those values, and something to draw while the program works.

```echo
command::Command $app = command::Command('search', '0.1.0', 'find a pattern');
command::Arg $file = $app->arg('file', 'f', '<path>', 'read from this file');
$app->require($file);

$r = $app->parse(env::argv());
```

That parse can refuse. [Getting started](/guide/) is the driver that handles the refusal, then the work.

```bash
epm add echolang/libcommand --git https://github.com/echolang/libcommand --range ^0.1
```

Echo sees the `command` namespace as soon as the module loads. The C shim rides along from libcommand's own manifest. You don't compile it yourself.
