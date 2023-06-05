#include <filesystem>
#include <string>
#include <fstream>

#if defined(__linux__) || defined(__OpenBSD__) || defined(__FreeBSD__)
#define RUNNER_SUPPORTED_POPEN
#include <stdio.h>
#elif defined(_WIN32)
#define RUNNER_SUPPORTED_WIN32
#include <windows.h>
#else
#error "No test runner implementation for this platform."
#endif

#if defined(RUNNER_SUPPORTED_WIN32)
int testprocess(const wchar_t* path)
#elif defined(RUNNER_SUPPORTED_POPEN)
int testprocess(const char* path)
#endif
{
	int exitcode = 1;
#if defined(RUNNER_SUPPORTED_WIN32)
	DWORD dwExitCode;
	STARTUPINFOW startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	//startupInfo.dwFlags &= STARTF_USESTDHANDLES;
	//startupInfo.hStdOutput = (HANDLE)-1;
	//startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	PROCESS_INFORMATION processInfo;
	BOOL success = CreateProcessW(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	if (!success)
	{
		fprintf(stderr, "Failed to create process (%S)\n", path);
		return 0;
	}
	HANDLE hProcess = processInfo.hProcess;
	WaitForSingleObject(hProcess, INFINITE);
	GetExitCodeProcess(hProcess, &dwExitCode);
	exitcode = dwExitCode;
#elif defined(RUNNER_SUPPORTED_POPEN)
	FILE* f = popen(path, "r");
	exitcode = pclose(f);
#endif
	return exitcode;
}

int main()
{
	for (auto& entry : std::filesystem::directory_iterator("./testbin"))
	{
		std::filesystem::path entrypath = std::filesystem::absolute(entry.path());
		int res = testprocess(entrypath.c_str());
		const auto exename = entry.path().filename().string();
		printf(
			"%s%s%s: Code %i for %s\n",
			res != 0 ? "\x1B[31m" : "\x1B[32m",
			res != 0 ? "FAIL" : "PASS",
			"\033[0m",
			res,
			exename.c_str());
	}
	return 0;
}