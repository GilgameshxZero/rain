// Stripped preprocessor defines for the STRINGIFY and STRINGIFY_INNER macros,
// for use in limited-syntax environments such as .rc.

// Macro for stringification. Use STRINGIFY(x) on variable x to stringify its
// value.
#ifndef STRINGIFY_INNER

#define STRINGIFY_INNER(x) #x
#ifndef STRINGIFY
#define STRINGIFY(x) STRINGIFY_INNER(x)
#endif

#endif
