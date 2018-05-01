#ifndef H_GEN_BUILD
#define H_GEN_BUILD

struct Build {
	static int GetVersionMajor();
	static int GetVersionMinor();
	static const char* GetVersionStr();
	static const char* GetGitCommit();

	template<typename F>
	static constexpr void DebugOnly(F&& fun) {
		fun();
	}

	static constexpr bool IsDebug() {
		bool r = false;
		DebugOnly([&] () {
			r = true;
		});
		return r;
	}
};

#endif