## Compile source and build binary
In each folder, run `make` or other suitable command based on the Makefile.

## Inspect the binary
- ELF info: `objdump -h MODULE_NAME.ko`
- ELF module related info: `objdump MODULE_NAME.ko -d -j .modinfo`
- Module info: `modinfo MODULE_NAME.ko`

## Install module
`sudo insmod MODULE_NAME.ko`

If there is param defined in the module, the param can be passed in commandline
by `sudo insmod MODULE_NAME.ko PARAM_1_NAME=PARAM_1_VALUE PARAM_2_NAME=PARAM_2_VALUE ...`

## Remove installed module
`sudo rmmod MODULE_NAME`

## List installed modules
`lsmod`
