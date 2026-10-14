# libcommand

a CLI utility library for [Echo](https://github.com/echolang/echo)

You are writing a command-line program. You need argv to become values, a `--help` page that stays true to those values, a few things to draw while the program works (a table, a tree, a progress bar, a spinner), and a list the user can pick from. That is the whole job.

```echo
command::Command $app = command::Command('search', '0.1.0', 'find a pattern');
command::Arg $file = $app->arg('file', 'f', '<path>', 'read from this file');
$app->require($file);

$r = $app->parse(env::argv());
if ($r->error != null) {
    command::Error $e = $r->error ?? .noCommand;
    command::CLIContext $err = .from($r->parsed, $app, .stderr);
    $err->print(command::renderError($e, $app, $r->parsed, $err));
    env::exit(1);
}
```

That parse can refuse. The guide is the rest of it: the table, the page, the widgets. It lives in [`docs/`](docs/guide/index.md), and the published site is <https://echolang.github.io/libcommand/>.

## Getting started

From your project directory:

```bash
epm add echolang/libcommand --git https://github.com/echolang/libcommand --range ^0.1
```

That is all. Echo sees the `command` namespace as soon as the module loads. The C shim (raw mode, the seam a prompt needs) rides along from libcommand's own `#[cc: sources]`. You don't compile `c/posix.c` or `c/win32.c` yourself.

## Running the tests

```bash
echoc test
```

## Trying the examples

```bash
echoc run -m . examples/01-args.eco -- --file notes.txt TODO
echoc run -m . examples/02-subcommands.eco -- build -o app src/main.eco
echoc run -m . examples/03-help.eco -- --help
echoc run -m . examples/04-widgets.eco
echoc run -m . examples/05-live.eco
echoc run -m . examples/06-prompts.eco
echoc run -m . examples/07-session.eco
echoc run -m . examples/08-create-app.eco
echoc run -m . examples/09-logstream.eco
```

`06`, `07` and `08` take the numbered-list path when stdin is not a tty. You can feed answers with `printf '1\n' | echoc run -m . examples/06-prompts.eco`.
