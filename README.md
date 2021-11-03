# rcc - rCore in C

rCore-Tutorial-v3: https://github.com/rcore-os/rCore-Tutorial-v3

rCore-Tutorial-Book-v3 (Chinese): https://rcore-os.github.io/rCore-Tutorial-Book-v3/

## Dependency

- Toolchain: `riscv64-unknown-elf-*`

- Emulator: QEMU 5.0.0

## Getting Started

To select a certain chapter,

```
git tag
git checkout ch*
```

To build `rcc` kernel, enter `os` directory and then

```
make
```

To run rcc kernel in QEMU, enter `os` directory and then

```
make run
```

## Debug `rcc`

### Dependency

- Debugger: `gdb-multiarch` (`riscv64-unknown-elf-gdb` also works)

### Debug in Terminal

Enter `os` directory and then

```
make debug-run
```

In another terminal, enter `os` directory and then

```
make debug-gdb
```

### Debug in VSCode

Create `tasks.json` and `launch.json` under `.vscode` directory as follows.

```json
// tasks.json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "qemu debug-run",
      "type": "shell",
      "command": "echo Starting QEMU & cd ${workspaceFolder}/os && make debug-run",
      "isBackground": true,
      "problemMatcher": {
        "pattern": {
          "regexp": "^(Starting QEMU)",
          "line": 1,
        },
        "background": {
          "activeOnStart": true,
          "beginsPattern": "^(Starting QEMU)",
          "endsPattern": "^(Starting QEMU)"
        }
      }
    }
  ]
}

// launch.json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "rcc-kernel-debug",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/os/build/rcc.elf",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}/os",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "/usr/bin/gdb-multiarch",
      "miDebuggerServerAddress": "localhost:1234",
      "preLaunchTask": "qemu debug-run",
    }
  ]
}
```

## Reference

`rcc` project is not built from scratch. During the development of `rcc`, part of the source code or ideas is referred from other online courses or open source projects.

| Name              | Git                                               |
| -                 | -                                                 |
| rCore             | https://github.com/rcore-os/rCore                 |
| uCore             | https://github.com/DeathWish5/ucore-Tutorial      |
| xv6-riscv         | https://github.com/mit-pdos/xv6-riscv-fall19      |
| Abstract Machine  | https://github.com/NJU-ProjectN/abstract-machine  |
| TestOS            | https://github.com/ZimingYuan/testos              |
| glibc             | https://sourceware.org/git/glibc.git              |
| printf            | https://github.com/mpaland/printf                 |
| musl              | http://git.etalabs.net/cgit/musl                  |
| Elfparse          | https://github.com/DBarthe/Elfparse               |
