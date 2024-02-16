# T8X disassembly and analsysis plugin

Toshiba 8X MCU were used in various Toyota ECUs in 80s - 90s.
It is based on Motorola M68HC11, however have slightly different
instruction set.

### Install

To build package rizin, pkg-config, ninja-build, python3 are required.

Building the plugin

```bash
meson setup builddir
cd builddir
meson compile
meson install
```

The plugin is installed to rizin user plugin dir.

Run following command to find out user plugin dir for rizin:

```bash
rizin -H RZ_USER_PLUGINS
```

### Verify

Verify if plugin install was successful:

```bash
$ rz-asm -L | grep t8x
_dA_  8          t8x         MIT     Toshiba 8X disassembly plugin
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
