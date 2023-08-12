// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "detours.lib")

#include "AirVehicle.h"
#include "astruct.h"

typedef void* (__fastcall CameraManipulationFunction)(AirVehicle*, DWORD, DWORD, astruct*, DWORD, DWORD);
CameraManipulationFunction* origCameraManipulationFunction = NULL;
AirVehicle* lastAirVehiclePtr = NULL;
wchar_t msgbuf[1024] = {};

extern struct CameraCoordsPacket cc;

float mapYawAngleToXpos(float yaw, float minYawAngle, float maxYawAngle, float minXpos, float maxXpos)
{
    return (yaw - minYawAngle) * (maxXpos - minXpos) / (maxYawAngle - minYawAngle) + minXpos;
}

void headMovingHandlerInAirVehicle(AirVehicle* pAirVehicle) {
    if (lastAirVehiclePtr != pAirVehicle) {
        swprintf(msgbuf, sizeof(msgbuf), L"last airVehicle ptr: %p, new airVehicle ptr: %p\n", lastAirVehiclePtr, pAirVehicle);
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

        lastAirVehiclePtr = pAirVehicle;
    }

    D3DXMATRIX rotationMatrix;
    D3DXMatrixIdentity(&rotationMatrix);
    D3DXMatrixRotationYawPitchRoll(&rotationMatrix, D3DXToRadian(cc.yaw), D3DXToRadian(-cc.pitch), D3DXToRadian(cc.roll));

    D3DXMATRIX* m = (D3DXMATRIX*)pAirVehicle->pHeadMatrix;

    m->_11 = rotationMatrix._11;
    m->_12 = rotationMatrix._12;
    m->_13 = rotationMatrix._13;

    m->_21 = rotationMatrix._21;
    m->_22 = rotationMatrix._22;
    m->_23 = rotationMatrix._23;

    m->_31 = rotationMatrix._31;
    m->_32 = rotationMatrix._32;
    m->_33 = rotationMatrix._33;

    if (cc.yaw <= -20.0) {
        m->_41 = mapYawAngleToXpos(cc.yaw, -179.0f, -20.0f, -0.35f, 0.0f);
    } else if (cc.yaw >= 20.0) {
        m->_41 = mapYawAngleToXpos(cc.yaw, 20.0f, 179.0f, 0.0f, 0.35f);
    } else {
        m->_41 = 0.0f;
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
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)udpServer, NULL, 0, NULL);

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
        stopUdpServer();
        Sleep(1000);
    }

    return TRUE;
}

