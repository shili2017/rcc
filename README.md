# rcc - rCore in C

rCore-Tutorial-v3: https://github.com/rcore-os/rCore-Tutorial-v3

## Dependency

1. Toolchain: `riscv64-unknown-elf-*`

1. QEMU 5.0.0

## Getting Started

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

1. Debugger: `gdb-multiarch` (`riscv64-unknown-elf-gdb` also works)

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
