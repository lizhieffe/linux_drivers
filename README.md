## Compile source and build binary
In each folder, run `make` or other suitable command based on the Makefile.

## Inspect the binary
- ELF info: `objdump -h MODULE_NAME.ko`
- Module related info: `objdump MODULE_NAME.ko -d -j .modinfo`

## Install module
`sudo insmod MODULE_NAME.ko`

## Remove installed module
`sudo rmmod MODULE_NAME`

## List installed modules
`lsmod`
