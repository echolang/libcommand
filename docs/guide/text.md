---
description: A typed line, with the editing keys you expect, a hint, and a fallback.
---

# Text

You need a path, a name, a port: an answer nobody can list in advance. [](api:command::element::Text) reads one line.

```echo
$p = $ctx->create<element::Text>('Where should we create it?');
$p->hint('./my-app');
$p->otherwise('./my-app');
$dir = $p->ask();
```

<!-- render: text-hint -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Where should we create it?</span>
<span class="line">  <span style="background:#e6edf5;color:#0a0e15">.</span><span style="opacity:.55">/my-app</span></span></code></pre></div>
<!-- /render -->

The field is empty. What you see is the hint, dimmed, with the caret sitting on its first character.

`hint` and `otherwise` are two ideas on purpose. I don't want a placeholder that quietly becomes the answer. A hint is a suggestion drawn in the empty field. A fallback is the value an empty line means. Most of the time they're the same string, and then you call both.

## Typing

The caret is drawn, not moved to: reverse video over the character it sits on. Here is `./tools/cli` typed, then the left arrow three times.

<!-- render: text-typed -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Where should we create it?</span>
<span class="line">  ./tools/<span style="background:#e6edf5;color:#0a0e15">c</span>li</span></code></pre></div>
<!-- /render -->

The editing is what you'd expect from a shell. Backspace and delete, left and right, home and end, Ctrl-W for the last word and Ctrl-U for the line. Everything moves a whole scalar at a time, so `ä` never breaks in half.

A line longer than the row scrolls sideways. The window only moves when the caret leaves it, so the text doesn't slide under you on every keystroke.

## After enter

<!-- render: text-done -->
<div class="code surface" data-lang="terminal"><div class="code-head"><span class="code-file"><span class="dot"></span>terminal</span></div><pre class="code-body"><code style="font-family:Menlo,Consolas,&quot;DejaVu Sans Mono&quot;,monospace;line-height:1.2;color:#e6edf5"><span class="line">  Where should we create it?  <span style="opacity:.55">./tools/cli</span></span></code></pre></div>
<!-- /render -->

The field collapses to the title and the answer. With an empty line and a fallback, the answer is the fallback.

## The short spelling

```echo
$name = element::text($ctx, 'What is your name?');
$dir = element::text($ctx, 'Where should we create it?', './my-app');
```

The three-argument form sets the hint and the fallback to the same string, then asks. The one-argument form sets neither. When the two should differ, mint a `Text` and call the two methods.

## An empty answer

Set neither a hint nor a fallback and an empty line is a refusal: enter does nothing on a tty, and on a pipe the call dies. `require(false)` makes an empty string a fine answer. `require` starts on, same as a [Select](./select.md).

## On a pipe

The call prints the title, with the fallback in brackets when there is one, and reads a line.

```text
  Where should we create it? [./my-app]
```

A trailing carriage return is dropped, so a line from a Windows file is the same answer.

`examples/06-prompts.eco` asks this exact question.
