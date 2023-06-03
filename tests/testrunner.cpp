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
	STARTUPINFOW startupInfo { 0 };
	PROCESS_INFORMATION processInfo;
	BOOL success = CreateProcessW(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	if (!success)
	{
		fprintf(stderr, "Failed to create process (%s)\n", path);
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

int main(int argc, char* argv[])
{
	for (auto& entry : std::filesystem::directory_iterator("./testbin"))
	{
		std::filesystem::path entrypath = std::filesystem::absolute(entry.path());
		int res = testprocess(entrypath.c_str());
		printf("Code %i for %s\n", res, entry.path().filename().string().c_str());
	}
	return 0;
}