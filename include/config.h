#define UseScheduler 1

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#ifndef FWVersionMajor
#define FWVersionMajor 0
#endif
#ifndef FWVersionMinor
#define FWVersionMinor 0
#endif
#ifndef FWVersionBuild
#define FWVersionBuild 0
#endif
#define FWVersion ((2 * 10000) + (2 * 1000) + (0))
#define FWVERSION_STR STRINGIFY(FWVersionMajor) "." STRINGIFY(FWVersionMinor) "." STRINGIFY(FWVersionBuild)
#define FWVERSION_EXT_STR FWVERSION_STR "-" GITHASH
