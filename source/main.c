#include <inttypes.h>
#include <stdio.h>
#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include "sha1.h"



static void waitInput(const u16 keys)
{
	do
	{
		VBlankIntrWait();
		scanKeys();
	} while(!(keysDown() & keys));
}

__attribute__((section(".iwram"), long_call, target("arm"))) static void hashRom(void)
{
	puts(CON_CLS() "Hashing ROM. This takes about 1 minute...\n");
	SHA1_CTX ctx;
	u8 sha1[20];
	SHA1Init(&ctx);
	SHA1Update(&ctx, (void*)0x08000000u, 1024u * 1024 * 32);
	SHA1Final(sha1, &ctx);
	puts("SHA1 (0x8000000-0x9FFFFFF):");
	for(u32 i = 0; i < 20; i++)
	{
		iprintf("%02" PRIX8, sha1[i]);
	}
	puts("");

	waitInput(KEY_B);
}

__attribute__((section(".iwram"), long_call, target("arm"))) static void checkRomPadding(void)
{
	extern const u32 __rom_end__[];

	iprintf(CON_CLS() "Checking ROM padding (0x%08" PRIX32 "-0x9FFFFFF)...\n\n", (u32)__rom_end__);
	const vu32 *ptr;
	for(ptr = (vu32*)__rom_end__; ptr < (vu32*)0xA000000; ptr++)
	{
		if(*ptr != 0x55555555u) break;
	}

	if(ptr != (vu32*)0xA000000)
	{
		iprintf("Mismatch at address 0x%08" PRIX32 ".\n", (u32)ptr);
		iprintf("Expected 0x55555555, got 0x%08" PRIX32 ".\n", *ptr);
	}
	else puts("No mismatches found.");

	waitInput(KEY_B);
}

int main(void)
{
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

	// Set SRAM and ROM wait states to the fastest supported by original games.
	// 3DS hardware supports down to 2 SRAM and 3/1 ROM wait states.
	*((vu16*)0x4000204) = 0x45B7; // SRAM 8 cycles, ROM0/1/2 3/1 cyles + prefetch enabled.

	while(1)
	{
		puts(CON_CLS() "Flashcart ROM test\n\n(A) Hash entire ROM (SHA1)\n(B) Check ROM padding");

		while(1)
		{
			VBlankIntrWait();
			scanKeys();

			const u16 kDown = keysDown();
			if(kDown & KEY_A) hashRom();
			else if(kDown & KEY_B) checkRomPadding();

			if(kDown) break;
		}
	}

	return 0;
}