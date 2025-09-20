/*
 * launcher.c
 * Native x86 windows dll injector and launcher.
 * Created: September 10, 2025
 * Author: radiomanV
 */

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <stdio.h>
#include <wchar.h>

// Dll injector function
static BOOL inject_dll(HANDLE hProc, const wchar_t *dllPath)
{
	SIZE_T size = (wcslen(dllPath) + 1) * sizeof(wchar_t), wr = 0;
	LPVOID remote = VirtualAllocEx(
		hProc, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!remote)
		return FALSE;
	BOOL ok = WriteProcessMemory(hProc, remote, dllPath, size, &wr) &&
		  wr == size;
	if (!ok) {
		VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
		return FALSE;
	}

	HMODULE hm = GetModuleHandleW(L"kernel32.dll");
	FARPROC pLoadLib = hm ? GetProcAddress(hm, "LoadLibraryW") : NULL;
	if (!pLoadLib) {
		VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
		return FALSE;
	}

	HANDLE thrd = CreateRemoteThread(hProc, NULL, 0,
					 (LPTHREAD_START_ROUTINE)pLoadLib,
					 remote, 0, NULL);
	if (!thrd) {
		VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
		return FALSE;
	}

	WaitForSingleObject(thrd, INFINITE);
	DWORD ret = 0;
	GetExitCodeThread(thrd, &ret);
	CloseHandle(thrd);
	VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
	return ret != 0;
}

// Check file path
static int check_path(const wchar_t *base_dir, const wchar_t *arg, wchar_t *out,
		      size_t out_sz)
{
	if (!arg || !*arg)
		return 0;

	if (!PathIsRelativeW(arg)) {
		wcsncpy(out, arg, out_sz);
		out[out_sz - 1] = L'\0';
	} else {
		if (!PathCombineW(out, base_dir, arg))
			return 0;
	}

	DWORD att = GetFileAttributesW(out);
	return (att != INVALID_FILE_ATTRIBUTES) &&
	       !(att & FILE_ATTRIBUTE_DIRECTORY);
}

// Main function
int wmain(int argc, wchar_t **argv)
{
	const wchar_t *exe_arg = L"";
	const wchar_t *shim_arg = L"shim.dll";
	int forced = 0;

	// Get the launcher's directory
	wchar_t dir[MAX_PATH];
	DWORD n = GetModuleFileNameW(NULL, dir, ARRAYSIZE(dir));
	if (n && n < ARRAYSIZE(dir)) {
		PathRemoveFileSpecW(dir);
	}

	// Parse command line
	int pf = -1;
	for (int i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--exe") == 0 && i + 1 < argc) {
			exe_arg = argv[++i];
			forced = 1;
			continue;
		}
		if (wcscmp(argv[i], L"--shim") == 0 && i + 1 < argc) {
			shim_arg = argv[++i];
			continue;
		}
		if (wcscmp(argv[i], L"--") == 0) {
			pf = i + 1;
			break;
		}
	}

	wchar_t exe_path[MAX_PATH], shim_path[MAX_PATH];
	if (forced) {
		if (!check_path(dir, exe_arg, exe_path, ARRAYSIZE(exe_path))) {
			wprintf(L"[launcher] --exe path not found or invalid: \"%ls\"\n",
				exe_arg);
			return EXIT_FAILURE;
		}
	} else {
		const wchar_t *progs[] = { L"Minipro.exe", L"Xgpro.exe",
					   L"Xgpro_T76.exe" };
		int found = 0;
		for (size_t i = 0; i < ARRAYSIZE(progs); ++i) {
			if (check_path(dir, progs[i], exe_path,
				       ARRAYSIZE(exe_path))) {
				exe_arg = progs[i];
				found = 1;
				break;
			}
		}
		if (!found) {
			wprintf(L"[launcher] --exe not specified and no known executable found.\n",
				dir);
			return EXIT_FAILURE;
		}
	}

	// Resolve shim (accept absolute or relative)
	if (!check_path(dir, shim_arg, shim_path, ARRAYSIZE(shim_path))) {
		wprintf(L"[launcher] Shim not found: \"%ls\" (base: \"%ls\")\n",
			shim_arg, dir);
		return EXIT_FAILURE;
	}

	// Build the target process command line
	wchar_t cmdline[4096];
	int off = _snwprintf(cmdline, ARRAYSIZE(cmdline), L"\"%s\"", exe_path);
	if (off < 0 || off >= (int)ARRAYSIZE(cmdline)) {
		wprintf(L"[launcher] Command line too long (exe).\n");
		return EXIT_FAILURE;
	}
	if (pf > 0) {
		for (int i = pf; i < argc; ++i) {
			int wrote = _snwprintf(cmdline + off,
					       ARRAYSIZE(cmdline) - off,
					       L" \"%ls\"", argv[i]);
			if (wrote < 0 ||
			    off + wrote >= (int)ARRAYSIZE(cmdline)) {
				wprintf(L"[launcher] Command line too long.\n");
				return EXIT_FAILURE;
			}
			off += wrote;
		}
	}

	// Create a suspended process
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pinfo = { 0 };
	if (!CreateProcessW(NULL, cmdline, NULL, NULL, FALSE,
			    CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT, NULL,
			    NULL, &si, &pinfo)) {
		wprintf(L"[launcher] CreateProcessW failed, err=%lu\n",
			GetLastError());
		return EXIT_FAILURE;
	}

	// Inject our shim.dll into the target process
	if (!inject_dll(pinfo.hProcess, shim_path)) {
		wprintf(L"[launcher] DLL injection failed, err=%lu\n",
			GetLastError());
		TerminateProcess(pinfo.hProcess, 1);
		CloseHandle(pinfo.hThread);
		CloseHandle(pinfo.hProcess);
		return EXIT_FAILURE;
	}

	// Resume process and wait for exit
	ResumeThread(pinfo.hThread);
	WaitForSingleObject(pinfo.hProcess, INFINITE);
	DWORD ret = 0;
	GetExitCodeProcess(pinfo.hProcess, &ret);
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	return (int)ret;
}
