---
description: Select, Text, and Confirm. A tty gets keys. A pipe gets a line.
---

# Prompts

There are three. [](api:command::element::Select) is a list, [](api:command::element::Text) is a typed line, [](api:command::element::Confirm) is a yes or no. Each is minted from the context, and `ask()` already knows where to draw.

On a terminal the prompt is a multi-row region. It climbs with cursor-up, which is a different protocol from the trailing line a spinner uses. A prompt and a live `Spinner` or `Progress` on the same stream will fight over the cursor. Finish one before you start the other, or put the live work inside a [session](./session.md), where the channel owns the row.

Ctrl-C is a signal. It kills the program. Raw mode is refcounted, and the tty is restored on the way out, including from that signal.

`examples/06-prompts.eco` is the three of them, on their own.

## A list

`ask` on a Select always returns `array<string>`. Radio is length 1.

```echo
$fruit = element::select($ctx, 'Pick a fruit', $fruits);

$q = $ctx->create<element::Select>('Which features?', element::SelectKind::checks);
$q->choice('Logging');
$q->choice('Caching');
$q->pick('Logging');
$on = $q->ask();
```

`element::select` is the short spelling: title, choices, and radio unless you pass `SelectKind::checks`. The long way is `choice` (one label) or `choices` (the array), then `ask`.

On a tty, arrow keys move, space toggles checks, enter confirms. After confirm the menu collapses to a summary.

On a pipe, the same call prints a numbered list and reads a line. `2` picks radio item 2. `1,3` or `1 3` picks checks. An empty line uses `pick` if you set one, otherwise it is a refusal.

`pageSize` is how many rows the menu shows before it scrolls. `wrap` is whether the cursor leaves the last item and lands on the first. `require(false)` is a menu that may confirm with nothing marked. `require` starts on.

## A line

A `Text` reads a line, with the editing you would expect: backspace and delete, left and right, home and end, Ctrl-W for the last word and Ctrl-U for the line. Everything moves a whole scalar at a time, so `ä` never breaks in half.

```echo
$p = $ctx->create<element::Text>('Where should we create it?');
$p->hint('./my-app');
$p->otherwise('./my-app');
$dir = $p->ask();
```

`hint` and `otherwise` are two ideas on purpose. I don't want a placeholder that quietly becomes the answer. A hint is a suggestion drawn in the empty field. A fallback is the value an empty line means.

The three-argument `element::text($ctx, $title, $placeholder)` sets both to the same string, then asks. When they should differ, mint a `Text` and call the two methods. The one-argument `element::text` sets neither.

Set neither and an empty line is a refusal, unless you call `require(false)`. `require` starts on, same as a Select.

## A yes or no

```echo
$go = element::confirm($ctx, 'Continue?');
```

On a tty, left and right move, `y` and `n` answer outright, and enter confirms. The highlighted answer starts on yes. On a pipe, the call prints the question and reads a line: anything starting with `y` is yes, anything starting with `n` is no.

`words` renames the two answers when yes and no are the wrong pair for the question.

```echo
$q = $ctx->create<element::Confirm>('Overwrite?');
$q->words('overwrite', 'keep');
bool $wipe = $q->ask();
```
