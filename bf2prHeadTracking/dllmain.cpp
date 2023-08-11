// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "detours.lib")

#include "AirVehicle.h"
#include "astruct.h"

typedef void* (__fastcall CameraManipulationFunction)(AirVehicle*, DWORD, DWORD, astruct*, DWORD, DWORD);
CameraManipulationFunction* origCameraManipulationFunction = NULL;
AirVehicle* lastAirVehicle = NULL;
wchar_t msgbuf[1024] = {};

void headMovingHandlerInAirVehicle(AirVehicle* pAirVehicle) {
    if (lastAirVehicle != pAirVehicle) {
        swprintf(msgbuf, sizeof(msgbuf), L"last airVehicle ptr: %p, new airVehicle ptr: %p\n", lastAirVehicle, pAirVehicle);
        OutputDebugString(msgbuf);
        swprintf(msgbuf, sizeof(msgbuf), L"ptr to head position matrix: %p\n", pAirVehicle->pHeadMatrix);
        OutputDebugString(msgbuf);
        D3DXMATRIX* m = (D3DXMATRIX*)pAirVehicle->pHeadMatrix;
        swprintf(msgbuf, sizeof(msgbuf), L"%4.2f, %4.2f, %4.2f, %4.2f\n%4.2f, %4.2f, %4.2f, %4.2f\n%4.2f, %4.2f, %4.2f, %4.2f\n%4.2f, %4.2f, %4.2f, %4.2f\n",
                    m->_11, m->_12, m->_13, m->_14,
                    m->_21, m->_22, m->_23, m->_24,
                    m->_31, m->_32, m->_33, m->_34,
                    m->_41, m->_42, m->_43, m->_44);
        OutputDebugString(msgbuf);

        lastAirVehicle = pAirVehicle;
    }
}

void __fastcall myCameraManipulationFunction(AirVehicle* pAirVehicle, DWORD edx, DWORD p1, astruct* p2, DWORD p3, DWORD p4) {

    if (pAirVehicle->field431_0x1d0 != 0x6) {
        if (pAirVehicle->field20_0x14->field740_0x2e4 != 0x0) {
            if (! (((p2->field250_0x100 & 0x800) != 0x0) && (0.5 < p2->field41_0x2c)) )  {
                headMovingHandlerInAirVehicle(pAirVehicle);
            }
        }
    }

    origCameraManipulationFunction(pAirVehicle, edx, p1, p2, p3, p4);
}

void* getAddressRelativeToModule(const wchar_t* moduleName, const DWORD offset) {
    return reinterpret_cast<unsigned char*>(GetModuleHandle(moduleName)) + offset;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        origCameraManipulationFunction = (CameraManipulationFunction*)getAddressRelativeToModule(L"PRBF2.exe", 0x1A3530);

        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)origCameraManipulationFunction, myCameraManipulationFunction);
        DetourTransactionCommit();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)origCameraManipulationFunction, myCameraManipulationFunction);
        DetourTransactionCommit();
    }

    return TRUE;
}

