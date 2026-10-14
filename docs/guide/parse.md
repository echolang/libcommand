---
description: parse turns argv into stated values, and keeps the partial parse on a refusal.
---

# Parsing

`parse` turns argv into stated values. Nothing here is settled: `--color auto` is still the word `"auto"`.

```echo
$r = $app->parse(env::argv());
```

[](api:command::Outcome) is two fields. `$parsed` is the table filled in as far as the walk got. `$error` is null when the parse is good.

Here is the catch: the outcome carries the partial parse even on refusal, so a bad flag still takes the colour the invocation asked for. Unwrap with `$r->error ?? .noCommand`. `"{$e}"` is the refusal sentence. That interpolation is `str::from` on [](api:command::Error), which calls `message()`.

## The four branches

So, what happens if this fails? The driver is four branches: refusal, version, help, work.

```echo
if ($r->error != null) {
    command::Error $e = $r->error ?? .noCommand;
    command::CLIContext $err = .from($r->parsed, $app, .stderr);
    $err->print(command::renderError($e, $app, $r->parsed, $err));
    env::exit(1);
}

if ($r->parsed->wantsVersion) {
    io::println(command::renderVersion($app));
    env::exit(0);
}

if ($r->parsed->wantsHelp) {
    command::CLIContext $out = .from($r->parsed, $app, .stdout);
    $out->print(command::renderHelp($app, $r->parsed, $out));
    env::exit(0);
}
```

`--help` and `--version` short-circuit the remaining rules, including required flags. `greet build --help` prints the page rather than complaining about a missing `-o`.

Help goes to stdout. A refusal goes to stderr. The pages those two calls draw are [Help](./help.md).

## Reading it back

[](api:command::Parsed) is the filled table. You read a row through the handle you kept when you built it.

```echo
$path = $parsed->value($out);
$have = $parsed->stated($out);
array<string> $all = $parsed->values($repeated);
```

`value` is the word, or the fallback when the row has one and the user said nothing. `stated` is whether this invocation wrote the option. `values` is every word a `repeat` row collected. Positionals are `$parsed->positionals`, in order. Words after a `--` are `$parsed->tail`.

`$parsed->subject($app)` is the command name when there is one, and the program name when the words never got that far. Refusals use it so the sentence can say who was missing an argument.

`$parsed->wantsHelp` and `$parsed->wantsVersion` are the short-circuits. When help named an option, `$parsed->helpTopic` is that bare word (`optimize`, no dashes) and the page for it is what `renderHelp` draws.

The whole driver is `examples/01-args.eco`.
