---
description: Install libcommand and run the four-branch driver.
---

# Getting started

You have a program, and argv is still an array of words. libcommand turns that array into values, and it draws the page that explains the same values.

The namespace is `command`. Widgets live in `command::element`. One module load is the whole install. `isatty` and the window width already live on `std::io::stream`. Colour policy (`NO_COLOR`, `COLUMNS`) stays in Echo. The C shim, raw mode for a prompt, is compiled in from libcommand's own `#[cc: sources]`.

## Install it

From your project directory:

```bash
epm add echolang/libcommand --git https://github.com/echolang/libcommand --range ^0.1
```

## A complete pass

Let's build a program that takes one pattern and one required file. The pieces have somewhere to sit after that. The same program is `examples/01-args.eco`.

```echo
use command;
use std::env;
use std::io;

command::Command $app = command::Command('search', '0.1.0', 'find a pattern');
$app->positional('<pattern>', 'the pattern', 'A regular expression.', 1, 1, '');
command::Arg $file = $app->arg('file', 'f', '<path>', 'read from this file');
$app->require($file);
```

`Command` is the table. `positional` is a word with no dash. `arg` is a row that takes a value, and the handle it returns is how you read that value back. `require` with no command means the root invocation has to state it.

Now parse. Nothing in the result is settled past "the user wrote this word".

```echo
$r = $app->parse(env::argv());
```

So, what happens if this fails? The driver is four branches: refusal, version, help, work. A refusal still carries the partial parse, which is how the error page can honour the colour the invocation asked for.

```echo
if ($r->error != null) {
    command::Error $e = $r->error ?? .noCommand;
    command::CLIContext $err = .from($r->parsed, $app, .stderr);
    $err->print(command::renderError($e, $app, $r->parsed, $err));
    env::exit(1);
}

$parsed = $r->parsed;

if ($parsed->wantsVersion) {
    io::println(command::renderVersion($app));
    env::exit(0);
}

if ($parsed->wantsHelp) {
    command::CLIContext $help = .from($parsed, $app, .stdout);
    $help->print(command::renderHelp($app, $parsed, $help));
    env::exit(0);
}

echo "file: {$parsed->value($file)}";
echo "pattern: {$parsed->positionals[0]}";
```

Help goes to stdout. A refusal goes to stderr. `$parsed->value($file)` takes the handle, not the string `"file"`, so it cannot look up a name the table does not have.

Run it the way the example does:

```bash
echoc run -m . examples/01-args.eco -- --file notes.txt TODO
echoc run -m . examples/01-args.eco -- --help
```

The table itself is [the next page](./command.md). The four branches, and what a partial parse still knows, are [Parsing](./parse.md).

## This site

The pages you are reading are this repo's `docs/` folder, built by [ecodoc](https://github.com/echolang/ecodoc). The workflow that publishes them is `.github/workflows/docs.yml`.

The coloured terminal windows on the widget pages are not screenshots. `gen/` is a small program in this module that draws every widget into a buffer context and pastes the HTML between the `<!-- render: ... -->` markers. Change how a widget looks, then run it again. `--check` writes nothing and exits 1 when a page is out of date.

```bash
echoc run --target render
echoc run --target render -- --check
``` One setting lives in the repository, not in git: Settings, Pages, Source: GitHub Actions.
