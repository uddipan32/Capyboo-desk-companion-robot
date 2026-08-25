# Capyboo Serial Protocol

Host API for talking to the Capyboo desk companion over USB serial.  
Use this document to build a Windows app / “driver” (user-mode serial client), not a kernel driver.

---

## Connection

| Setting | Value |
|--------|--------|
| Interface | USB virtual COM port (ESP32 USB-UART) |
| Baud rate | **115200** |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |
| Line ending | Every command must end with `\n` (LF). `\r\n` also works. |

### Windows notes

1. After plugging in, Windows assigns a COM port (e.g. `COM3`).
2. Opening the port may reset the ESP32 (DTR). Wait ~2–3 seconds after open before sending commands.
3. Prefer reading replies asynchronously (device also prints status and debug lines).
4. Suggested client stack: .NET `System.IO.Ports.SerialPort`, or Win32 `CreateFile` + `ReadFile`/`WriteFile` on `\\.\COMx`.

### Ready signal

On boot the device prints lines ending with something like:

```text
Ready (serial only).
```

Wait for this (or a timeout) before sending commands.

---

## Message framing

- **One command = one line**
- Encoding: ASCII / UTF-8 plain text (printable characters)
- Commands are matched case-insensitively for keywords; **label text** in `progress:` and display strings keeps the casing you send where noted
- Max practical line length: **256 characters**
- Device replies with text lines (also newline-terminated)

### Write example

```text
mode:progress\n
progress:42:Build:Compiling...\n
```

### Read example

```text
Received: progress:42:Build:Compiling...
Progress 42% | Build | Compiling...
```

---

## Modes

Switch with:

```text
mode:<name>
```

| Mode value | Description |
|------------|-------------|
| `horse` | Galloping horse; speed follows `cpu:` (default when serial host connected) |
| `animation` | Face animations (default when serial host not connected) |
| `weather` | Weather screen (data via `weather:` commands) |
| `game` | Dino game (`jump` to start / jump) |
| `clock` | Clock display (set with `time:`) |
| `progress` | Progress bar + top/bottom text |

Examples:

```text
mode:horse
mode:animation
mode:weather
mode:game
mode:clock
mode:progress
```

Typical replies:

```text
Switched to Horse mode
Switched to Animation mode
Switched to Weather mode
Switched to Game mode. Send 'jump' to play.
Switched to Clock mode
Switched to Progress mode
```

---

## Command reference

### `ping`

Health check.

| | |
|--|--|
| Format | `ping` |
| Effect | OLED shows `pong` briefly; serial replies `pong` |
| Reply | `pong` |

```text
ping
```

---

### `cpu:<percent>`

Updates host CPU load for Horse mode. Also switches to Horse mode.

| | |
|--|--|
| Format | `cpu:<0-100>` |
| Effect | Sets gallop speed (0% = slow, 100% = fast) and enters `mode:horse` |
| Reply | `CPU <n>%` |

Examples:

```text
cpu:12
cpu:87
```

---

### `mode:<name>`

See [Modes](#modes).

---

### `mood:<name>`

Sets animation mood (best used in `animation` mode).

| | |
|--|--|
| Format | `mood:<name>` |
| Effect | Selects that animation sequence immediately if in animation mode |

**Supported moods**

| Name | Meaning |
|------|---------|
| `idle` | Idle look around |
| `happy` | Happy sequence |
| `enjoying` | Enjoying |
| `angry` | Angry |
| `sad` | Sad |
| `verysad` | Very sad + tears |
| `cry` | Crying |
| `funny` | Funny / tongue |
| `love` | Love (then returns to random) |
| `sleep` | Sleepy |
| `thumbup` | Thumbs up |
| `wave` | Wave |
| `random` | Random sequences (default behavior) |

Example:

```text
mood:happy
```

Reply:

```text
Mood set to: happy
```

---

### `weather:<city>:<temp>:<feels>:<humidity>:<description>`

Pushes weather data for Weather mode. Does **not** fetch from the internet; the Windows app should fetch weather and send this.

| Field | Type | Notes |
|-------|------|--------|
| city | string | Required |
| temp | float | °C |
| feels | float | Feels like °C |
| humidity | int | Percent |
| description | string | Optional (e.g. `clear sky`) |

Examples:

```text
weather:London:18:16:65:cloudy
weather:Mumbai:32:35:70
```

Reply on success:

```text
Weather data updated
```

Then show it:

```text
mode:weather
```

---

### `time:<HH:MM:SS>` or `time:<HH:MM:SS> <DD/MM/YYYY>`

Sets the on-device clock.

Examples:

```text
time:14:30:00
time:14:30:00 28/07/2026
```

Replies:

```text
Clock time set successfully
```

or

```text
Invalid clock format. Use: time:HH:MM:SS or time:HH:MM:SS DD/MM/YYYY
```

---

### `message:<text>`

Shows arbitrary text on the OLED (overrides normal mode drawing while non-empty).

Examples:

```text
message:Hello from Windows
message:
```

Sending `message:` with no text clears the overlay and returns to the current mode UI.

---

### `progress:<percent>[:<bottom>]` or `progress:<percent>:<top>:<bottom>`

Progress mode UI:

```text
┌────────────────────────────┐
│         <top text>         │
│      [████████░░░░]        │
│            75%             │
│       <bottom text>        │
└────────────────────────────┘
```

Also **auto-switches** to Progress mode.

| Format | Meaning |
|--------|---------|
| `progress:50` | Set 0–100 only |
| `progress:50:Please wait` | Percent + bottom text |
| `progress:75:Syncing:Uploading files` | Percent + top + bottom |

Rules:

- Percent is clamped to **0–100**
- Top/bottom are truncated on screen to ~21 characters
- Colons separate fields; avoid extra `:` inside top/bottom labels

Examples:

```text
mode:progress
progress:0:Idle:Ready
progress:25:Build:Compiling
progress:100:Done:Success
```

Reply example:

```text
Progress 25% | Build | Compiling
```

---

### `jump`

Dino game: start game, jump, or restart after game over.  
Use while in `mode:game`.

```text
mode:game
jump
```

---

### `tickle`

Plays tickle animation (works from any mode; blocks briefly while playing).

```text
tickle
```

---

### `loveyou`

Plays love-you animation (blocks briefly while playing).

```text
loveyou
```

---

## Suggested Windows driver / host API

Map high-level host methods to serial lines:

| Host method | Serial command |
|-------------|----------------|
| `Connect(port)` | Open COM @ 115200, wait for `Ready` |
| `Ping()` | `ping` → expect `pong` |
| `SetMode(mode)` | `mode:{mode}` |
| `SetCpu(percent)` | `cpu:{percent}` |
| `SetMood(mood)` | `mood:{mood}` |
| `SetWeather(...)` | `weather:{city}:{temp}:{feels}:{humidity}:{desc}` |
| `SetTime(dt)` | `time:HH:MM:SS` or with date |
| `ShowMessage(text)` | `message:{text}` |
| `SetProgress(pct, top, bottom)` | `progress:{pct}:{top}:{bottom}` |
| `Jump()` | `jump` |
| `Tickle()` | `tickle` |
| `LoveYou()` | `loveyou` |

### Pseudocode (.NET)

```csharp
serial.BaudRate = 115200;
serial.NewLine = "\n";
serial.DtrEnable = true;   // may reset board on open
serial.Open();
Thread.Sleep(2500);        // wait for boot + Ready

serial.WriteLine("ping");
string reply = serial.ReadLine(); // look for "pong" among lines

serial.WriteLine("cpu:42");
```

### Discovery tip

Enumerate ports with friendly names containing `USB`, `CP210`, `CH340`, `Silicon Labs`, or `ESP`. Store last successful COM port in app settings.

---

## Response handling

Every accepted command is echoed as:

```text
Received: <original line>
```

Then a command-specific status line may follow.  
Ignore unknown debug lines (e.g. random sequence indices). Treat protocol as **line-oriented**, not binary.

Recommended host reader loop:

1. Read lines continuously on a background thread
2. Match known prefixes (`Received:`, `pong`, `CPU `, `Progress `, `Switched to`, `Mood set`, `Weather data`, `Clock time`, `Invalid`)
3. Raise events for UI

---

## Error cases

| Situation | Device behavior |
|-----------|-----------------|
| Unknown `mode:` | OLED error text + serial help line |
| Bad `weather:` | Serial `Invalid weather format: ...` |
| Bad `time:` | Serial invalid format message |
| No newline / incomplete line | Command may fire after ~80 ms idle timeout |
| Line longer than 256 chars | Buffer discarded |

---

## Quick test checklist (Serial Monitor or host)

```text
ping
cpu:25
cpu:90
mode:animation
mood:wave
mode:clock
time:12:00:00
mode:weather
weather:Tokyo:22:21:55:clear
mode:progress
progress:10:Welcome:Underground Editor
progress:50:Working:Halfway
progress:100:Done:Complete
mode:game
jump
```

---

## Firmware source

Protocol implemented in: `firmware/capyboo/capyboo.ino`  
Display: 128×64 SSD1306 OLED over I2C.
