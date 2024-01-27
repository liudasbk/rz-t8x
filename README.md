# T8X disassembly plugin

Toshiba 8X dissasembly plugin for rizin

### Install

Make sure you have rizin installed. Make uses `pkg-config` to find
lib and header file locations.

```bash
make
make install
```

### Verify

Verify if plugin install was successful:

```bash
$ rz-asm -L | grep t8x
_d__  8          t8x         MIT     Toshiba 8X disassembly plugin
```

### Usage example

Using `rz-asm` command:

```bash
$ rz-asm -d -a t8x ca01cb0108
ld a, 0x01
ld b, 0x01
add a, b
```

Using `rizin`:

```bash
[0x00000000]> e asm.arch=t8x
[0x00000000]> o ./demo.bin 
[0x00000000]> pd 3
            0x00000000      ld    a, 0x01
            0x00000002      ld    b, 0x01
            0x00000004      add   a, b
```
