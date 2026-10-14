---
description: Command is the table. Parse, usage, help and every refusal read it.
---

# The table

[](api:command::Command) is the table. Adding a flag is adding a row. Parse, usage, the help page and every refusal read those rows, so the page cannot drift from the parser.

```echo
command::Command $app = command::Command('greet', '0.1.0', 'say hello');
command::Subcommand $build = $app->command('build', 'compile a native executable', 'Compile it.');
$app->positional($build, '<sources...>', 'the files', 'Loose files become the main module.', 0, -1, 'What is built');

command::Arg $out = $app->arg('output', 'o', '<file>', 'where the executable is written');
$app->category($out, 'What is built');
$app->require($out, $build);
array<string> $outProse = [
    'The path of the binary.',
    '  greet build -o app src/*.eco',
];
$app->describe($out, $outProse);
```

`require($out, $build)` also accepts the flag on that command, so the row is both required there and legal there. The same call with no subcommand is root-only.

`examples/02-subcommands.eco` is this table, end to end.

## Handles

A handle (`Subcommand`, `Flag`, `Arg`) is an index into the table. I don't want a string lookup that can silently miss.

```echo
$path = $parsed->value($out);
$have = $parsed->stated($out);
```

`$out` is a thing the table already knows. `$parsed->value($out)` can't look up the wrong name, because there is no name.

## Positionals

A program with no subcommands takes positionals on the root. Same table, no `command()`:

```echo
$app->positional('<pattern>', 'the pattern', 'A regular expression.', 1, 1, '');
```

The two integers are min and max. The default, if you never call `positional`, is min 0 and max 0, so an undeclared word is a refusal. `max = -1` means unlimited. An empty category string means the row sits with the rest of the options, under no extra heading.

`$app->tail(false)` refuses a leftover `--`. The default is to accept one, and the words after it land in `$parsed->tail`.

## Values

A closed set of values is `OptionValue(name, summary, description)`. You hand the set to the row with `values`. `valueOn` is how one name in that set is restricted to one command:

```echo
$app->values($color, $colors);
$app->valueOn($opt, 'whole', $build);
```

That is a mark on the value, not a bitmask you compute at the call site.

## The rows that are already there

`--help`, `--version` and `--color` are registered in the constructor. `--colour` is an alias on the same row, so `$parsed->stated($app->color)` is true for either spelling. The three values are `auto`, `always` and `never`, and the fallback is `auto`.

Global options may appear before the command. A command-scoped option waits until the command is known. A value that is only a dash and a digit (`-1`) is a value, not a flag.

## Why it is a class

`Command` is a class on purpose. Copying a half-built table and adding to both copies would desynchronise the handles. After you finish building, parse and help only take `const Command&`.

The other builder calls are the same idea as `describe` and `require`:

- `flag` is a row with no value. `arg` takes one. `repeat` takes the value again each time it is stated.
- `alias` adds a second long name on the same row. `--colour` is that.
- `fallback` is the word `value()` returns when the option was not stated.
- `accept` makes a row legal on one command, without making it required.
- `exclude` is two rows that answer one question. Stating both is a refusal.
- `category` is the heading the compact help page files the row under.

Each of those returns nothing, or a handle. The prose you pass to `describe` is what `--help <option>` prints. One string, or an array of lines when you want the breaks yourself.
