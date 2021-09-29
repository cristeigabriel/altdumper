# altdumper ![CPP](https://img.shields.io/badge/MADE%20WITH-C%2B%2B-blue) ![PLATFORM](https://img.shields.io/badge/PLATFORM-WINDOWS-blue)
Simple, fully external, smart, fast, JSON-configurated, **feature-rich** Windows x86 DLL Memory Dumper with **Code Generation**. Written in Modern C++.

## Features
- Fully external (Binary based)
  <details>

  - You're not required to run any program other than altdumper to generate your values from a config. You just need according binaries.
  </details>
- Code generation
  <details>

  - Your config output automatically generates to valid, no-cost (compile-time) variables.
  - This component can be used independently.
  ---
  - Currently supported languages are:
    - C++
  ---
  </details>
- JSON-configurated
  <details>

  - The inputs are very human friendly (meaning, you can edit your configs by hand) and ideal for usage on a server.
  - This also makes it pretty portable, without any official support for editing, as it is incredibly intuitive.
  </details>
- Fully user assisting
  <details>

  - You will be walked through both the process of generating a JSON configuration for making, and throughout inputting it. The process is a dialogue, and you'll have file/folder prompts when required, name inputting when required, value inputting when required, and you'll also have instructions at hand, in the CLI.
  </details>
- Multi-threaded
  <details>

  - The processing of every individual DLL is spanned across their own thread.
  </details>
- Pattern scanning
  <details>

  ---
  - Keep in mind: Scanning here is done only throughout the **.text** section.
  ---

  Prompts you to input the following:

  - IDA-style string of pattern (example: "**AA BB CC DD EE ? FF**").
    - **?** is the 'ignore mismatch' wildcard.
  - N-th instance of pattern (given it repeats).
    - Default value is 0 (first one).
  - Padding (from first pattern byte).
  - Dereferences (from pattern start + padding).

  </details>
- String-search scanning
  <details>
  Prompts you to input the following:

  - String to find in **.rdata**. Input is null terminated.
  - Reference instance (N-th reference in **.text** of the address where our string is stored).
  - Padding (to skip over reference pointer, you would input 4).
  - Dereferencing (from padding).
  </details>
- Misc scanning
  <details>
  
  ---
  - Work in progress.
  ---
  </details>

## License
[WTFPL](https://github.com/cristeigabriel/altdumper/blob/main/LICENSE) 