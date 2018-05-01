#ifndef H_GEN_BUILD
#define H_GEN_BUILD

struct Build {
	static int GetVersionMajor();
	static int GetVersionMinor();
	static const char* GetVersionStr();
	static const char* GetGitCommit();
};

#endif