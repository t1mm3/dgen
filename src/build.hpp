#ifndef H_GEN_BUILD
#define H_GEN_BUILD

struct Build {
	static int GetVersionMajor();
	static int GetVersionMinor();
	static const char* GetVersionStr();
	static const char* GetGitCommit();
	static const char* GetTypeStr();
};


#ifdef __GNUC__
#define NO_INLINE __attribute__ ((noinline))
#else
#define NO_INLINE
#endif

#endif