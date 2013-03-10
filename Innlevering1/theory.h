#ifndef theory_H
#define theory_H

/************************************************************************
* http://www.gamedev.net/topic/359794-c-direct3d-hooking-sample/ 
*
*
*
*************************************************************************/

// Calculate offset
DWORD from_int = reinterpret_cast<DWORD> (address_d3d9_Present);

DWORD to_int = reinterpret_cast<DWORD> (&PresentHook);

// This version of the JMP instruction takes an address relative
// to the current address, and it is 5 bytes long
// So the relative offset is ‘to - from - 5'
// Don’t worry about the unsigned DWORD underflowing
DWORD offset = to_int - from_int - 5;


// Assemble the patch at the beginning of Present
const unsigned char jmp = 0xE9; // The opcode for a 32-bit rel JMP


unsigned char* ip = reinterpret_cast<unsigned char*> (address_d3d9_Present);

*ip = jmp;

*(reinterpret_cast<DWORD*> (ip + 1)) = offset;


HRESULT __cdecl PresentHook(
	const RECT* pSourceRect,
	const RECT* pDestRect,
	HWND hDestWindowOverride,
	const RGNDATA* pDirtyRegion)

{

	IDirect3dDevice9* device;

	__asm {MOV device, ECX}



	// Do anything that needs to be done before Present gets called



	// Remove the Present hook
	HRESULT return_value = Present(pSourceRect,

		pDestRect,

		hDestWindowOverride,

		pDirtyRegion);

	// Reinstall the Present hook


	// Do anything that needs to be done after Present gets called
	return return_value;

}

#endif // theory_H