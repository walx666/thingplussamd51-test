Import("env", "projenv")
import os

# General options that are passed to the C and C++ compilers
#env.Append(CCFLAGS=["flag1", "flag2"])

def after_buildfs(source, target, env):
    print ("Post build scripts")

#env.AddPostAction("$BUILD_DIR/firmware.bin", after_buildfs)

# Build .uf2 from .bin
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
#    env.AddPostAction("$BUILD_DIR/firmware.bin", after_buildfs)
    env.VerboseAction(" ".join([
        "python", 
        "./tools/uf2conv.py", "-f 0x68ed2b88 -b 0x2000","$BUILD_DIR/${PROGNAME}.bin","-o $BUILD_DIR/${PROGNAME}.uf2"
    ]), "Building $BUILD_DIR/${PROGNAME}.uf2")
)

# Custom .hex from .elf
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", "-O", "ihex", "-R", ".eeprom",
        "$BUILD_DIR/${PROGNAME}.elf", "$BUILD_DIR/${PROGNAME}.hex"
    ]), "Building $BUILD_DIR/${PROGNAME}.hex")
)