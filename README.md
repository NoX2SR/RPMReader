# RPMReader

`RPMReader` is the timing-sensitive rotational sensor node. It samples an engine optocoupler and a drive-shaft reed switch, calculates their pulse frequencies, and returns those frequencies to the motorcycle monitor over half-duplex RS-485.

It exists as a separate Arduino project so deterministic pulse sampling is isolated from gear, temperature, display, and communication work in the companion [GearSensor](https://github.com/NoX2SR/GearSensor) and [MainMonitor](https://github.com/NoX2SR/MainMonitor) projects.

## How measurement works

Timer2 is configured directly in compare-match mode at 8 kHz. The interrupt service routine samples both input pins on every tick:

1. A low level arms the channel for the next rising transition.
2. At the rising transition, the elapsed tick count is converted to hertz with `8000 / ticks`.
3. If no new transition is measured for more than 1000 ticks, the frequency is set to zero. At 8 kHz this is approximately 125 ms.

The node reports frequency, not final dashboard units. `MainMonitor` currently multiplies the first channel by 60 for engine RPM and applies the existing magnet, drive-ratio, and tire-size formula to the second channel for speed.

## Hardware and pins

Target: Arduino Uno / ATmega328P. The direct Timer2 register access makes the target MCU significant.

| Pin | Purpose | Current interpretation |
| --- | --- | --- |
| D2 | Engine optocoupler | Sampled digital pulse input |
| D3 | Drive-shaft reed switch | Sampled digital pulse input |
| D7 | Debug output | Configured but not currently driven |
| D8 | RS-485 DE/RE | Low receives, high transmits |
| D0/D1 | Hardware serial | 115200 baud |

## Serial protocol

The implemented request byte is `Q`. A response starts with `Q:` and contains two frequency channels. Each float is sent twice for receiver-side comparison:

```text
Q:<freq1 bytes>:<freq1 bytes>:<freq2 bytes>:<freq2 bytes>::
```

Each value is the four-byte AVR `float` representation with every byte bitwise inverted. The format is binary despite using ASCII `:` separators. Cases for commands `W` and `R` exist in the response function, but the current command parser accepts only `Q`, so those cases cannot be requested externally.

## Build and test

On Debian or Ubuntu, install the AVR toolchain if needed:

```bash
sudo apt install arduino-builder arduino-core-avr gcc-avr avr-libc
```

Then run:

```bash
make verify
```

Firmware artifacts are written to `build/firmware/`. Native tests cover Timer2 setup, command parsing, binary response encoding, both response channels, pulse measurement, and zero-frequency timeout. Current production line coverage is 98.92%.

## Wokwi simulation

The repository includes a Wokwi `diagram.json` and `wokwi.toml`. Two clock generators stand in for the conditioned sensor outputs:

- D2 receives a 100 Hz engine pulse signal.
- D3 receives a 20 Hz drive-shaft pulse signal.
- LEDs expose the currently unused D7 debug output and D8 RS-485 transmit-enable output.
- A logic analyzer records UART RX/TX, DE/RE, both pulse inputs, and debug output.

Build the firmware, open `diagram.json`, and run `Wokwi: Start Simulator`:

```bash
make build
```

Send `Q` in the serial terminal to request both measured frequencies. The response contains binary bytes, so it will not look like ordinary readable text; use the logic-analyzer VCD or a protocol decoder when inspecting it. Change the clock generators' `frequency` attributes in `diagram.json` to simulate other speeds.

The serial terminal is attached directly to the Uno UART. Wokwi does not model the MAX485 differential bus in this diagram; D8 only drives the visible transmit-enable LED.

## Editor setup

The checked-in `.vscode` configuration points IntelliSense at the installed AVR compiler, Arduino core, and Uno variant. It also associates `.ino` files with C++. Open this repository directly, or use `Motorcycle.code-workspace` in the parent directory for all three repositories.

## Current limitations

- Sampling uses `digitalRead()` inside an 8 kHz interrupt, which is simple but relatively expensive on AVR.
- Shared ISR values are not currently declared `volatile` or copied atomically before use in the main loop.
- The binary payload can contain the `:` separator byte; there is no escaping, length field, checksum, or explicit frame delimiter.
- The protocol depends on AVR byte order and four-byte IEEE-754 floats.
- The 125 ms zero timeout limits measurement of very low pulse frequencies.
- Input conditioning, switch bounce, optocoupler behavior, electrical noise, and motorcycle transient protection require hardware validation.
