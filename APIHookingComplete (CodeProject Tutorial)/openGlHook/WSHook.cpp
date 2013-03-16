#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

//#include <gl/glaux.h>
#include <detours.h>
#include <iostream>
#include <fstream>
#include <math.h>

typedef void      (APIENTRY*FUNC_GLBEGIN)			(GLenum mode); 

FUNC_GLBEGIN				x3x_glBegin			=	 NULL;

void APIENTRY Hooking_glBegin(GLenum mode)
{


	//-- As you can see we do the code BEFORE the original OGL glBegin function is called
	x3x_glBegin(mode);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD rfcall, LPVOID lpReserved)
{ 
	switch (rfcall) 
	{
	case DLL_PROCESS_ATTACH: {
		DisableThreadLibraryCalls((HMODULE)hModule); 
		x3x_glBegin = (FUNC_GLBEGIN) DetourFunction((PBYTE)DetourFindFunction("OpenGl32.dll", "glBegin"), (PBYTE)Hooking_glBegin); 
							 }
							 break; 
							 //  Follow example below

							 //    DetourRemove((PBYTE)x3x_glFunc,(PBYTE)Hooking_glBe  gin); ---- This is where we are removing the redirection to 
							 //    allow the OGL glBegin to pass through BUT now with our code inside...


	case DLL_PROCESS_DETACH:
		DetourRemove((PBYTE)x3x_glBegin,(PBYTE)Hooking_glB  egin); 
		break;
	}
	return TRUE;