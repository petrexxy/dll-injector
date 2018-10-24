#include<stdio.h>
#include<Windows.h>
#include<TlHelp32.h>
#include<stdlib.h>

#define true 1
#define false 0 //TimeToInject Function returns 'true' or 'false' and function is type int - Converted keywords

#ifndef USAGE
#define USAGE "USAGE: inject.exe [DLL PATH] [-execute (Program To Execute)] || [-process (Process Name)] || [-window (Window Title)]\n"
#endif

DWORD GetProcIDByCreation(char* __file_location, HANDLE* __resume_thread);
DWORD GetProcIDByProcessName(char* __process_name);
DWORD GetProcIDByWindowTitle(char* __window_title);
int TimeToInject(DWORD __process_id, char* __dll_location);

int main(int argc, char** argv) {
	DWORD __proc_id;
	HANDLE hThread;
	char choice;

	if (argc != 4) {
		fprintf(stdout, "%s\n", USAGE); system("PAUSE");
	}

	choice = tolower(argv[2][1]);
	do {
		switch (choice) {
		case 'e': // argv[2] = -execute then argv[2][1] = 'e'
			__proc_id = GetProcIDByCreation(argv[3], &hThread);
			break;
		case 'p':
			__proc_id = GetProcIDByProcessName(argv[3]);
			break;
		case 'w':
			__proc_id = GetProcIDByWindowTitle(argv[3]);
			break;
		default:
			printf("Unknown Option %c\n", choice);
			fprintf(stderr, "%s\n", USAGE); system("PAUSE");
		}
	} while (!__proc_id);

	fprintf(stdout, "Process ID: %ld - Selected\n", __proc_id);
	printf("Injecting DLL...\n");
	printf("Injection: %s\n", TimeToInject(__proc_id, argv[1]) ? "Success" : "Failed");
	if (choice == 'c') {
		ResumeThread(hThread);
	}
};

int TimeToInject(DWORD __process_id, char* __dll_location) {
	HANDLE hProc;
	HANDLE hThread;
	LPVOID dllParamAddress;
	LPVOID loadLibrary;
	BOOL memoryWritten;

	loadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (!loadLibrary) { printf("Failed @ GetProcAddress\n"); return false; }

	hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, __process_id);
	if (!hProc) { printf("Failed @ OpenProcess\n"); return false; }

	dllParamAddress = VirtualAllocEx(hProc, 0, strlen(__dll_location), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!dllParamAddress) { printf("Failed @ VirtualAllocEx\n"); CloseHandle(hProc); return false; }

	memoryWritten = WriteProcessMemory(hProc, dllParamAddress, __dll_location, strlen(__dll_location), NULL);
	if (!memoryWritten) { printf("Failed @ WriteProcessMemory\n"); CloseHandle(hProc); return false; }

	hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)loadLibrary, dllParamAddress, 0, 0);
	if (!hThread) { printf("Failed @ CreateRemoteThread"); CloseHandle(hProc); return false; }

	CloseHandle(hThread);
	CloseHandle(hProc);
	return true;
}

DWORD GetProcIDByWindowTitle(char* __window_title) {
	HWND hWind;
	DWORD __proc_id = 0;
	hWind = FindWindow((NULL || 0), __window_title);
	if (hWind) { GetWindowThreadProcessId(hWind, &__proc_id); }
	return __proc_id;
}

DWORD GetProcIDByProcessName(char* __process_name) {
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	HANDLE hProcSnap;
	DWORD __process_id = 0;
	hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(hProcSnap, &pe32)) {
		do {
			if (!strcmp(pe32.szExeFile, __process_name)) {
				__process_id = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hProcSnap, &pe32));
	}
	CloseHandle(hProcSnap);
	return __process_id;
}

DWORD GetProcIDByCreation(char* __file_location, HANDLE* __resume_thread) {
	STARTUPINFO __startup_info;
	PROCESS_INFORMATION __process_info;
	memset(&__startup_info, 0, sizeof(__startup_info));
	memset(&__process_info, 0, sizeof(__process_info));
	__startup_info.cb = sizeof(__startup_info);

	if (CreateProcess(NULL, __file_location, 0, 0, false, CREATE_SUSPENDED, 0, 0, &__startup_info, &__process_info)) {
		*__resume_thread = __process_info.hThread;
		return __process_info.dwProcessId;
	}
	return 0;
}