---
description: What Select, Text and Confirm share. A tty gets keys. A pipe gets a line.
---

# Prompts

There are three:

- A [Select](./select.md) is a list, with one answer or many.
- A [Text](./text.md) is a typed line.
- A [Confirm](./confirm.md) is a yes or no.

Each is minted from the context, and `ask()` already knows where to draw. Each one has a short spelling too, a free function that mints it and asks in one call.

```echo
$fruit = element::select($ctx, 'Pick a fruit', ['Apple', 'Banana', 'Orange']);
$dir = element::text($ctx, 'Where should we create it?', './my-app');
$go = element::confirm($ctx, 'Continue?');
```

## A tty gets keys

On a terminal the prompt is a multi-row region. It climbs with cursor-up and repaints on every key, which is a different protocol from the trailing line a spinner uses.

So a prompt and a live `Spinner` or `Progress` on the same stream will fight over the cursor. Finish one before you start the other, or put the live work inside a [session](./session.md), where the channel owns the row.

While a prompt is live, the terminal's own cursor is hidden: the prompt draws its own caret into the frame. Raw mode is refcounted, and the tty is restored on the way out.

Ctrl-C is a signal. It kills the program, and the cursor and the tty mode are put back first.

## A pipe gets a line

`ask()` runs live only when the stream is a terminal and stdin is one too. Otherwise the same call prints the question and reads one line from stdin. Same call, same return type, so a script can drive your wizard:

```bash
printf '1\n1\nmy-app\ny\n' | echoc run -m . examples/06-prompts.eco
```

Each widget page says what that line means for it. An answer the prompt can't accept is a refusal, and the program dies with a message instead of guessing.

## After the answer

An answered prompt collapses to a summary row, the title and the answer, so the scrollback reads as a transcript of what was asked and what was said. Inside a [session](./session.md) the answer gets its own row under the title.

`examples/06-prompts.eco` is the three of them, on their own.
