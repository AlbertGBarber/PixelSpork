#if !defined(max)
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#if !defined(min)
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#if !defined(SIZE)
#define SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif