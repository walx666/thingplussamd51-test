@echo off
cd "%CD%\..\.pio\build\sparkfun_samd51_micromod"
rem python ..\..\..\tools\uf2conv.py -f 0x55114460 -b 0x2000 .\firmware.bin -o .\firmware.uf2
python ..\..\..\tools\uf2conv.py -f 0x55114460 .\firmware.hex -o .\firmware.uf2
pause