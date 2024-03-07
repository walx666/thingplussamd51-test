import subprocess

Import("env")

# Read value from other environments
config = env.GetProjectConfig()

env.Replace(PROGNAME="firmware_%s" % config.get("version","custom_fwversion"))
#env.Replace(PROGNAME="firmware_%s" % env.GetProjectOption("custom_fwversion"))


def get_firmware_specifier_build_flag():
    ret = subprocess.run(["git","show","-s","--format=%h"], stdout=subprocess.PIPE, text=True) #Uses only annotated tags
    #ret = subprocess.run(["git", "describe", "--tags"], stdout=subprocess.PIPE, text=True) #Uses any tags
    build_githash = ret.stdout.strip()
    build_flag = "-D GITHASH=\\\"" + build_githash + "\\\""
    print ("Firmware Revision: " + build_githash)
    return (build_flag)

env.Append(
    BUILD_FLAGS=[get_firmware_specifier_build_flag()]
)