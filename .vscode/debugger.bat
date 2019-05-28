set PATH=C:\Users\Mateusz\.vscode\HusarionTools\bin\;%PATH%
cd m:\fork-kaiser || exit 1
start /wait st-flash write myproject.bin 0x08010000 || exit 1
start st-util
arm-none-eabi-gdb %*