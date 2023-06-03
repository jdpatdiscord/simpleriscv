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

int testprocess(const char* path)
{
	int exitcode = 1;
#if defined(RUNNER_SUPPORTED_WIN32)
	DWORD dwExitCode;
	STARTUPINFOA startupInfo { 0 };
	PROCESS_INFORMATION processInfo;
	HANDLE hProcess = CreateProcessA(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	WaitForSingleObject(hProcess);
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
		
		testprocess()
	}
	return 0;
}