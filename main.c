#include<stdio.h>
#include<Windows.h>
#include<TlHelp32.h>
#include<stdlib.h>

#define true 1
#define false 0 //TimeToInject Function returns 'true' or 'false' and function is type int - Converted keywords

#ifndef USAGE
#define USAGE "USAGE: inject.exe [DLL PATH] [-e (Program To Execute)] || [-p (Process Name)] || [-w (Window Title)]\n"
#endif

DWORD GetProcIDByCreation(char* __file_location, HANDLE* __res_thread);
DWORD GetProcIDByProcessName(char* __proc_name);
DWORD GetProcIDByWindowTitle(char* __window_title);
int TimeToInject(DWORD process_id, char* __dll_loc);

int main(int argc, char** argv) {
	DWORD proc_id;
	HANDLE handle_thread;
	char choice;

	if (argc != 4) {
		fprintf(stdout, "%s\n", USAGE); system("PAUSE");
	}

	choice = tolower(argv[2][1]);
	do {
		switch (choice) {
		case 'e': // argv[2] = -execute then argv[2][1] = 'e'
			proc_id = GetProcIDByCreation(argv[3], &handle_thread);
			break;
		case 'p':
			proc_id = GetProcIDByProcessName(argv[3]);
			break;
		case 'w':
			proc_id = GetProcIDByWindowTitle(argv[3]);
			break;
		default:
			printf("Unknown Option %c\n", choice);
			fprintf(stderr, "%s\n", USAGE); system("PAUSE");
		}
	} while (!proc_id);

	fprintf(stdout, "Process ID: %ld - Selected\n", proc_id);
	printf("Injecting DLL...\n");
	fprintf(stdout, "Injection: %s\n", TimeToInject(proc_id, argv[1]) ? "Success" : "Failed");
	if (choice == 'c') {
		ResumeThread(handle_thread);
	}
};

int TimeToInject(DWORD process_id, char* __dll_loc) {
	HANDLE handle_proc;
	HANDLE handle_thread;
	LPVOID dll_parameter_addr;
	LPVOID load_lib;
	BOOL mem_writ;

	load_lib = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (!load_lib) { printf("Failed @ GetProcAddress\n"); return false; }

	handle_proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
	if (!handle_proc) { printf("Failed @ OpenProcess\n"); return false; }

	dll_parameter_addr = VirtualAllocEx(handle_proc, 0, strlen(__dll_loc), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!dll_parameter_addr) { printf("Failed @ VirtualAllocEx\n"); CloseHandle(handle_proc); return false; }

	mem_writ = WriteProcessMemory(handle_proc, dll_parameter_addr, __dll_loc, strlen(__dll_loc), NULL);
	if (!mem_writ) { printf("Failed @ WriteProcessMemory\n"); CloseHandle(handle_proc); return false; }

	handle_thread = CreateRemoteThread(handle_proc, 0, 0, (LPTHREAD_START_ROUTINE)load_lib, dll_parameter_addr, 0, 0);
	if (!handle_thread) { printf("Failed @ CreateRemoteThread"); CloseHandle(handle_proc); return false; }

	CloseHandle(handle_thread);
	CloseHandle(handle_proc);
	return true;
}

DWORD GetProcIDByWindowTitle(char* __window_title) {
	HWND handle_window;
	DWORD proc_id = 0;
	handle_window = FindWindow((NULL || 0), __window_title);
	if (handle_window) { GetWindowThreadProcessId(handle_window, &proc_id); }
	return proc_id;
}

DWORD GetProcIDByProcessName(char* __proc_name) {
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	HANDLE handle_proc_snap;
	DWORD process_id = 0;
	handle_proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(handle_proc_snap, &pe32)) {
		do {
			if (!strcmp(pe32.szExeFile, __proc_name)) {
				process_id = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(handle_proc_snap, &pe32));
	}
	CloseHandle(handle_proc_snap);
	return process_id;
}

DWORD GetProcIDByCreation(char* __file_location, HANDLE* __res_thread) {
	STARTUPINFO startup_info;
	PROCESS_INFORMATION process_info;
	memset(&startup_info, 0, sizeof(startup_info));
	memset(&process_info, 0, sizeof(process_info));
	startup_info.cb = sizeof(startup_info);

	if (CreateProcess(NULL, __file_location, 0, 0, false, CREATE_SUSPENDED, 0, 0, &startup_info, &process_info)) {
		*__res_thread = process_info.hThread;
		return process_info.dwProcessId;
	}
	return 0;
}
