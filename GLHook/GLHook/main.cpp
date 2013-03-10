/****************************************************************************\
\*																			*/
/* For this tutorial I used a source code I found on planet-source-code.com *\
\* The only thing that actually remained the same is the HookGeneralFunction*/
/* the rest of this code was written by me. For further Copyrights and such,*\
\* look at the section below.												*/
/*																			*\
\*	This Source Code and it's tutorial were posted on the Extalia forum.	*/
/*		If you want to learn more, visit http://www.extalia.com/forums/		*\
\*																			*/
/*		Greetings															*\
\*			Tenebricosus													*/
/*																			*\
\****************************************************************************/


////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2004 Necr0Potenc3
// release date: January 25th, 2004
// dcavalcanti@bigfoot.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
////////////////////////////////////////////////////////////////////////////////

/* The includes */
#include <windows.h>						//Contains all the function declarations in the API
#include <stdio.h>							//Contains macro definitions, constants, and declarations of functions and types used for various standard input and output operations
#include "GLHook.h"						//Our own header

/* Function Prototypes */
	LONG WINAPI hook_ChangeDisplaySettings(LPDEVMODE lpDevMode, DWORD dwflags);
	HWND WINAPI hook_CreateWindowEx(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y,
								int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

/* Global Variable Declaration */
	DWORD CreateWindowExaddr=0;				//Global DWORD(4-bytes) to store the address of the CreateWindowEx API
	DWORD ChangeDisplaySettingsaddr=0;		//Global DWORD to store the addres of the ChangeDisplaySettings API

	BYTE backupCW[6];						//Array of bytes to save the original code when we hook the CreateWindowEx API
	BYTE backupCDS[6];						//Array of Bytes to save the original code when we hook ChangeDisplaySettings

/* The Dll's main function */
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)				//Check the different reasons for the call to this function
	{
		case DLL_PROCESS_ATTACH:				//This case is executed when the dll is attached to the process
		{
			DisableThreadLibraryCalls(hModule);					//keep the dll from being re-called
			MessageBox(0, "Attached!", "Dll Attach", 0);		//Let us know if the dll is loaded

			//Hook the necessary api's
			CreateWindowExaddr = HookGeneralFunction("user32.dll", "CreateWindowExA", hook_CreateWindowEx, backupCW);
			ChangeDisplaySettingsaddr = HookGeneralFunction("user32.dll", "ChangeDisplaySettingsA", hook_ChangeDisplaySettings, backupCDS);
			return true;						//If everything went according to plan, return true
		}break;
		case DLL_THREAD_ATTACH: break;			//We don't need these
		case DLL_THREAD_DETACH: break;			//We don't need these
		case DLL_PROCESS_DETACH:				//This case is executed when the dll gets detached(normally this would be when the process quits, but you don't know)
		{
			//restore the bytes (Unhook the process before detaching the dll, better save then sorry)
			if(CreateWindowExaddr) WriteProcessMemory((HANDLE)-1, (void*)CreateWindowExaddr, backupCW, 6, 0);
			if(ChangeDisplaySettingsaddr) WriteProcessMemory((HANDLE)-1, (void*)ChangeDisplaySettingsaddr, backupCDS, 6, 0);		
			MessageBox(0, "Dll Detached", "Process End", 0);
		}break;
	}
    return TRUE;							//If everything went right, return true
}

/* Hooked API Functions*/
HWND WINAPI hook_CreateWindowEx(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth,
						 int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
		dwExStyle			= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle				= WS_OVERLAPPEDWINDOW;							// Windows Style
		lpWindowName		= "Hooked!";									// We change the window name into "Hooked!" just for fun
		nWidth				= 500;											// We change the width, because we don't want a full screen window
		nHeight				= 700;											// We change the heigth
		ShowCursor(TRUE);													// Show Mouse Pointer

		/* Write the original code back(Unhook the API) */
		WriteProcessMemory((HANDLE)-1, (void*)CreateWindowExaddr, backupCW, 6, 0);
		//memcpy((void*)CreateWindowExaddr, backupCW, 6);
					MessageBox(0, "CreateWindowEx", "Process Info", 0);	
		/* Call the API but with the nessecary changes */
		HWND ret = CreateWindowEx(	dwExStyle,						// Extended Style For The Window
								lpClassName,						// Class Name
								lpWindowName,							// Window Title
								dwStyle|							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Defined Window Style
								x, 
								y,									// Window Position
								nWidth,								// Window Width
								nHeight,								// Window Height
								hWndParent,							// Parent Window
								hMenu,								// Menu
								hInstance,							// Instance
								lpParam);							// Pass Anything To WM_CREATE

		/* Reinstall the Hook */
		CreateWindowExaddr = HookGeneralFunction("user32.dll", "CreateWindowExA", hook_CreateWindowEx, backupCW);
		return ret;													// Return the Handle to the window we created
}


LONG WINAPI hook_ChangeDisplaySettings(LPDEVMODE lpDevMode, DWORD dwflags)
{
	return 0;														//DISP_CHANGE_SUCCESSFUL = 0, We return this value to let the hooked program think that everything went allright
}