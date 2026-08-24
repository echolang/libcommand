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
    $err->write(command::renderError($e, $app, $r->parsed, $err));
    env::exit(1);
}
```

That parse can refuse. The rest of this page is the table that made the refusal, the page that explains it, and the widgets you draw once argv is settled.

## Getting started

You don't need epm. Put this repository beside your project and name it from your `module.eco`:

```echo
#[module: "myapp"]
#[depends: "../libcommand"]
#[sources: "src/*.eco"]
```

That is all. Echo sees the `command` namespace as soon as the module loads. The C shim (raw mode, the seam a prompt needs) rides along from libcommand's own `#[cc: sources]`. You don't compile `c/posix.c` or `c/win32.c` yourself. `isatty` and the window width already live on `std::io::stream`; colour policy (`NO_COLOR`, `COLUMNS`) stays in Echo.

## Building a command

`command::Command` is the table. Parse, usage, the help page and every refusal read it. Adding a flag is adding a row.

```echo
command::Command $app = command::Command('greet', '0.1.0', 'say hello');
command::Subcommand $build = $app->command('build', 'compile a native executable', 'Compile it.');
$app->positional($build, '<sources...>', 'the files', 'Loose files become the main module.', 0, -1, 'What is built');

command::Arg $out = $app->arg('output', 'o', '<file>', 'where the executable is written');
$app->category($out, 'What is built');
$app->require($out, $build);
$app->describe($out, "The path of the binary.\n  greet build -o app src/*.eco");
```

`require($out, $build)` also accepts the flag on that command, so you don't have to write both.

### Handles

A handle (`Subcommand`, `Flag`, `Arg`) is an index into the table. I don't want a string lookup that can silently miss.

```echo
$path = $parsed->value($out);
$have = $parsed->stated($out);
```

`$out` is a thing the table already knows. `$parsed->value($out)` can't look up the wrong name, because there is no name.

### Values and positionals

A closed set of values is `OptionValue(name, summary, description)`. `$app->valueOn($opt, 'whole', $build)` is how a value is restricted to one command, not a bitmask you compute.

A program with no subcommands takes positionals on the root. Same table, no `command()`:

```echo
$app->positional('<pattern>', 'the pattern', 'A regular expression.', 1, 1, '');
command::Arg $file = $app->arg('file', 'f', '<path>', 'read from this file');
$app->require($file);
```

`require($file)` with no command is root-only. Positionals default to min 0 / max 0, so an undeclared word is a refusal. `max = -1` means unlimited. `$app->tail(false)` refuses a leftover `--`.

`--help`, `--version` and `--color` / `--colour` are registered for you. `--colour` is an alias on the same row, so `$parsed->stated($app->color)` is true for either spelling. Global options may appear before the command; command-scoped ones wait until the command is known. A value that is only a dash and a digit (`-1`) is a value, not a flag.

`Command` is a class on purpose. Copying a half-built table and adding to both copies would desynchronise the handles. After you finish building, parse and help only take `const Command&`.

`examples/01-args.eco` is the root-only program. `examples/02-subcommands.eco` is the greet/build table.

## Parsing

`parse` turns argv into stated values. Nothing here is settled: `--color auto` is still the word `"auto"`.

```echo
$r = $app->parse(env::argv());
```

Here is the catch: `Outcome` carries the partial parse even on refusal, so a bad flag still takes the colour the invocation asked for. `Error?` is null when the parse is good. Unwrap with `$r->error ?? .noCommand`. `"{$e}"` is the refusal sentence (`str::from` lives in `src/from.eco`).

So, what happens if this fails? The driver is four branches: refusal, version, help, work.

```echo
if ($r->error != null) {
    command::Error $e = $r->error ?? .noCommand;
    command::CLIContext $err = .from($r->parsed, $app, .stderr);
    $err->write(command::renderError($e, $app, $r->parsed, $err));
    env::exit(1);
}

if ($r->parsed->wantsVersion) {
    io::print(command::renderVersion($app));
    env::exit(0);
}

if ($r->parsed->wantsHelp) {
    command::CLIContext $out = .from($r->parsed, $app, .stdout);
    $out->write(command::renderHelp($app, $r->parsed, $out));
    env::exit(0);
}
```

`--help` and `--version` short-circuit the remaining rules, including required flags. `greet build --help` prints the page rather than complaining about a missing `-o`.

`--help optimize` (the bare name) is the prose page for one option. A leftover word that names no option still draws the compact page. Somebody who typed `greet build --help main.eco` asked for help.

Help goes to stdout. A refusal goes to stderr.

The whole driver is `examples/01-args.eco`. The pages it draws are `examples/03-help.eco`.

## Help

The page is generated from the table. There is deliberately no `--all`. I want the compact page to stay compact: one line per option. The paragraphs live one command further in.

```
greet build --help
greet build --help optimize
```

On a capable terminal the headings get a left bar and a dim rule, and value trees use the unicode branches from the theme. Colour paints command names, flag spellings and value names, and leaves the summaries alone. Colour and unicode fail apart: a CI log can have one and not the other.

`--version` prints the version string and a newline, nothing else.

`examples/03-help.eco` is the same table, aimed at those three invocations.

## The context

Widgets, help and refusals all need to know what they are drawing on. That pair (capabilities, stream) is a `CLIContext`. Name it once.

```echo
command::CLIContext $ctx = .resolve(.auto, .stderr);
```

`--color=always` / `never` win outright. `auto` honours `NO_COLOR`, `CLICOLOR_FORCE` and `TERM=dumb`, then asks isatty. Unicode is a locale question, separate from colour. Width 0 means unknown: help assumes 80 and caps at 100; a live line assumes 80 and still truncates.

`CLIContext::from($parsed, $app, $stream)` is the same resolution, honouring the `--color` the parse already took. A program that paints help on stdout and widgets on stderr makes two contexts. Tests mint `CLIContext::buffer($caps)` or `CLIContext::plain()` so nothing writes to the host tty.

## Widgets

A program that only wants a table never names `Command`. What it does name is a `CLIContext`, the terminal those widgets will draw on. Widgets live in `command::element`.

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

`create<T>` is a constructor call on T, with the context already in its hand. Echo has no variadics, so arity is overloads: `create<Spinner>()`, `create<Text>('What is your name?')`, `create<Select>('Which?', SelectKind::checks)`. At most two extra arguments. Two creates are two objects.

`$t->display()` writes through the context the widget was minted with, so the stream and colour decision was made once at `resolve` / `from`. `Table` and `Tree` also have `render()`, which returns the page as a string if you are composing. `display()` is how a widget leaves.

`$showHeader` and `$border` are public fields, not setters. `row()` dies if the row is longer than the columns. A `Tree` is built by pushing into `$node->children[]` directly.

`examples/04-widgets.eco` draws both.

## Live lines

Work is happening on this thread and you want a line that keeps moving. `Spinner` and `Progress` share one cursor protocol: a single trailing line, carriage return and erase-to-end-of-line, never cursor-up. The animation is the other thread, so the glyph keeps moving until you `succeed` / `fail` / `stop`.

```echo
$s = $ctx->create<element::Spinner>();
$s->start('fetching');
fetch();
$s->update('unpacking');
unpack();
$s->succeed('vendored 4 packages');
```

Or the short spelling:

```echo
$body = element::spin($ctx, 'fetching', function() : string {
    return fetch();
});
```

`element::spin` takes a `CLIContext`. There is no other overload. During a run it still joins the bar: the spinner is minted from that same context.

`Progress` is a cooperative bar. You `start` with a total, `advance` as work lands, `finish`. The fill moves only when you say something changed; a spinner on the row keeps moving until you commit. `advance` and `set` take a detail string. An empty string is fine.

On a pipe they do not rewrite: `paint` is silent and `commit` / `succeed` write a full line. A diagnostic on the same stream needs `suspend()` first, so the live row is erased.

`examples/05-live.eco` runs a spinner, the shorthand, a bar, and a `suspend()`.

## Prompts

There are three: `Select` is a list, `Text` is a typed line, `Confirm` is a yes or no. Each is minted from the context, and `ask()` already knows where to draw.

```echo
$fruit = element::select($ctx, 'Pick a fruit', $fruits);

$q = $ctx->create<element::Select>('Which features?', element::SelectKind::checks);
$q->choice('Logging');
$q->choice('Caching');
$q->pick('Logging');
$on = $q->ask();
```

`ask` on a Select always returns `array<string>`. Radio is length 1. On a TTY: arrow keys move, space toggles checks, enter confirms. The menu is a multi-row region (cursor-up), not a `LiveLine`. After confirm it collapses to a summary. A prompt and a live Spinner or Progress on the same stream will fight over the cursor.

On a pipe, the same call prints a numbered list and reads a line (`2` for radio, `1,3` or `1 3` for checks). An empty line uses `pick` if you set one, otherwise it is a refusal.

`pageSize`, `wrap` and `require` are the knobs. Search, groups and a markup mini-language are not here.

### Text

A `Text` reads a line, with the editing you would expect: backspace and delete, left and right, home and end, Ctrl-W for the last word and Ctrl-U for the line. Everything moves a whole scalar at a time, so a `ä` never breaks in half.

```echo
$p = $ctx->create<element::Text>('Where should we create it?');
$p->hint('./my-app');          // shown in the empty field, never the answer
$p->otherwise('./my-app');     // what an empty line means
$dir = $p->ask();

$go = element::confirm($ctx, 'Continue?');
```

`hint` and `otherwise` are two ideas on purpose. I don't want a placeholder that quietly becomes the answer. A hint is a suggestion the user can see and ignore; a fallback is a value. The three-argument `element::text($ctx, $title, $placeholder)` sets both to the same string. When they should differ, mint a `Text` and call the two methods. Set neither and an empty line is a refusal, unless you call `require(false)`.

`examples/06-prompts.eco` is the four of them, on their own.

## Sessions

A wizard is not a pile of prompts. `beginStepper` / `endStepper` on the context opens a connected run. Widgets look up the live channel when they draw, so a widget minted before the run still hangs off the bar once you open one. Dropping the context while a run is still open closes it too, which is why `endStepper` is how the last row gets to say something.

```echo
$ctx->beginStepper('create-app');

$dir = element::text($ctx, 'Where should we create it?', './my-app');
$type = element::select($ctx, 'Pick a project type', $types);
$go = element::confirm($ctx, 'Continue?');

$spin = $ctx->create<element::Spinner>();
$spin->start('writing files');
$spin->succeed('wrote 12 files');

$ctx->endStepper('Done');
```

```
┌  create-app
│
◇  Where should we create it?
│  ./my-app
│
◆  Pick a project type
│  ● TypeScript
│  ○ JavaScript
│  ↑/↓ to navigate • Enter: confirm
└
```

`◆` is the step waiting for you, `◇` one that answered. The channel is drawn in the colour of the step it belongs to, `└` included, and settles to grey the moment that step is answered, so the run reads as one lit stretch in a column of finished ones. `printError` during a run is a mark on that same channel; off a run it is the `error:` badge. `noteFrame` is the boxed aside that hangs off the bar.

The session is not a second factory. Prompts, spinners and progress bars are still minted from the context. `Spinner` and `Progress` join the bar when they are created during a run: their glyph moves into the bar column. `element::spin` has no separate session overload; it still takes a `CLIContext`.

A program that wants one prompt doesn't have to open a run to get it. Without a stepper every widget draws exactly as it always has, two spaces in.

Colour is still a capabilities question, now sitting on the context. Without it the run keeps its shape and loses its highlights, and without unicode the channel falls back to `| + > o`. One column either way, so nothing shifts.

`examples/07-session.eco` is the run. `examples/08-create-app.eco` is argv, then a session, then a summary table.

## What is not here yet

These are deliberate postponements, not accidents.

- Search and choice groups on a Select.
- Masked input. `TextBuffer` is there; nothing draws a `▪` per scalar yet.
- Validation callbacks on a prompt. The `▲` step symbol is in the theme waiting for them.
- Cancelling. I left `ISIG` alone on purpose. Ctrl-C is still a signal and still kills the program. There is no cancelled state to hand back. Raw mode is refcounted, and the tty is restored via atexit and signal handlers.
- A markup mini-language. Interpolation plus `command::styled` is the spelling.
- Panel, Rule, BarChart, Figlet, Canvas.
- Shell completion. The table is the data completions will want.
- Migrating epm onto this library.

## Running the tests

```bash
echoc test
```

Parser tests need no terminal. Help tests pin structure (every accepted option appears, empty categories stay gone, no line exceeds the page width), not wording. Live widgets and prompts write into a string buffer; keys are injected.

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
```

`06`, `07` and `08` take the numbered-list path when stdin is not a tty. You can feed answers with `printf '1\n' | echoc run -m . examples/06-prompts.eco`.

## Where things live

| File | What it holds |
|---|---|
| `c/posix.c`, `c/win32.c` | raw mode and one-byte reads (restored on Ctrl-C). isatty/columns are `std::io::stream` |
| `src/terminal.eco` | capabilities, SGR, `styled` |
| `src/context.eco` | `CLIContext`, the factory |
| `src/text.eco` | ASCII bytes, UTF-8 columns (one scalar is width 1) |
| `src/layout.eco` | wrap, headings, columns, tree branches |
| `src/theme.eco` | pretty / ascii glyphs. Colour is not in a theme. |
| `src/command.eco` | `Command`, options, handles |
| `src/parse.eco` | `Parser`, `Parsed`, `Outcome`. `$app->parse(argv)` |
| `src/error.eco`, `src/from.eco` | `Error`, and the `str::from` that makes `"{$e}"` work |
| `src/help.eco` | the page, one option, a refusal, `--version` |
| `src/element/step.eco`, `stepper.eco` | what a prompt owes the frame, and the channel down its left |
| `src/element/` | `Table`, `Tree`, `LiveLine`, `LiveRegion`, `Ticker`, `Progress`, `Spinner`, `Select`, `Text`, `Confirm` |

The namespace is `command`. Widgets are `command::element`.
