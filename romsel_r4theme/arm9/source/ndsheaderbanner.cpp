#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include "common/gl2d.h"

#include "ndsheaderbanner.h"
#include "module_params.h"

extern sNDSBannerExt ndsBanner;

// Needed to test if homebrew
char tidBuf[4];

typedef enum
{
	GL_FLIP_BOTH = (1 << 3)
} GL_FLIP_MODE_XTRA;

/**
 * Get the title ID.
 * @param ndsFile DS ROM image.
 * @param buf Output buffer for title ID. (Must be at least 4 characters.)
 * @return 0 on success; non-zero on error.
 */
int grabTID(FILE *ndsFile, char *buf)
{
	fseek(ndsFile, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	size_t read = fread(buf, 1, 4, ndsFile);
	return !(read == 4);
}

/**
 * Get SDK version from an NDS file.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return 0 on success; non-zero on error.
 */
u32 getSDKVersion(FILE *ndsFile)
{
	sNDSHeaderExt NDSHeader;
	fseek(ndsFile, 0, SEEK_SET);
	fread(&NDSHeader, 1, sizeof(NDSHeader), ndsFile);
	if (NDSHeader.arm7destination >= 0x037F8000 || grabTID(ndsFile, tidBuf) != 0)
		return 0;
	return getModuleParams(&NDSHeader, ndsFile)->sdk_version;
}

/**
 * Check if NDS game has AP.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return true on success; false if no AP.
 */
bool checkRomAP(FILE *ndsFile)
{
	char game_TID[5];
	grabTID(ndsFile, game_TID);
	game_TID[4] = 0;

	// Check for SDK4-5 ROMs that don't have AP measures.
	if ((memcmp(game_TID, "AZLJ", 4) == 0)  // Girls Mode (JAP version of Style Savvy)
	 || (memcmp(game_TID, "YEEJ", 4) == 0)  // Inazuma Eleven (J)
	 || (memcmp(game_TID, "VSO",  3) == 0)  // Sonic Classic Collection
	 || (memcmp(game_TID, "B2D",  3) == 0)  // Doctor Who: Evacuation Earth
	 || (memcmp(game_TID, "BWB",  3) == 0)  // Plants vs Zombies
	 || (memcmp(game_TID, "BRFP", 4) == 0)	 // Rune Factory 3 - A Fantasy Harvest Moon
	 || (memcmp(game_TID, "TFB",  3) == 0)  // Frozen: Olaf's Quest
	 || (memcmp(game_TID, "B88",  3) == 0)) // DS WiFi Settings
	{
		return false;
	}
	else
	// Check for ROMs that have AP measures.
	if ((memcmp(game_TID, "B", 1) == 0)
	 || (memcmp(game_TID, "T", 1) == 0)
	 || (memcmp(game_TID, "V", 1) == 0)) {
		return true;
	} else {
		static const char ap_list[][4] = {
			"ABT",	// Bust-A-Move DS
			"YHG",	// Houkago Shounen
			"YWV",	// Taiko no Tatsujin DS: Nanatsu no Shima no Daibouken!
			"AS7",	// Summon Night: Twin Age
			"YFQ",	// Nanashi no Geemu
			"AFX",	// Final Fantasy Crystal Chronicles: Ring of Fates
			"YV5",	// Dragon Quest V: Hand of the Heavenly Bride
			"CFI",	// Final Fantasy Crystal Chronicles: Echoes of Time
			"CCU",	// Tomodachi Life
			"CLJ",	// Mario & Luigi: Bowser's Inside Story
			"YKG",	// Kindgom Hearts: 358/2 Days
			"COL",	// Mario & Sonic at the Olympic Winter Games
			"C24",	// Phantasy Star 0
			"AZL",	// Style Savvy
			"CS3",	// Sonic and Sega All Stars Racing
			"IPK",	// Pokemon HeartGold Version
			"IPG",	// Pokemon SoulSilver Version
			"YBU",	// Blue Dragon: Awakened Shadow
			"YBN",	// 100 Classic Books
			"YVI",	// Dragon Quest VI: Realms of Revelation
			"YDQ",	// Dragon Quest IX: Sentinels of the Starry Skies
			"C3J",	// Professor Layton and the Unwound Future
			"IRA",	// Pokemon Black Version
			"IRB",	// Pokemon White Version
			"CJR",	// Dragon Quest Monsters: Joker 2
			"YEE",	// Inazuma Eleven
			"UZP",	// Learn with Pokemon: Typing Adventure
			"IRE",	// Pokemon Black Version 2
			"IRD",	// Pokemon White Version 2
		};

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(ap_list)/sizeof(ap_list[0]); i++) {
			if (memcmp(game_TID, ap_list[i], 3) == 0) {
				// Found a match.
				return true;
				break;
			}
		}

	}
	
	return false;
}

// bnriconframeseq[]
static u16 bnriconframeseq[64] = {0x0000};

// bnriconframenum[]
int bnriconPalLine = 0;
int bnriconframenumY = 0;
int bannerFlip = GL_FLIP_NONE;

// bnriconisDSi[]
bool isDirectory = false;
int bnrRomType = 0;
bool bnriconisDSi = false;
int bnrWirelessIcon = 0; // 0 = None, 1 = Local, 2 = WiFi
bool isDSiWare = false;
int isHomebrew = 0; // 0 = No, 1 = Yes with no DSi-Extended header, 2 = Yes with DSi-Extended header

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence()
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[i] = ndsBanner.dsi_seq[i];
	}
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence()
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[i] = 0x0000;
	}
}

static u16 bannerDelayNum = 0x0000;
int currentbnriconframeseq = 0;

/**
 * Play banner sequence.
 * @param binFile Banner file.
 */
void playBannerSequence()
{
	if (bnriconframeseq[currentbnriconframeseq] == 0x0001 && bnriconframeseq[currentbnriconframeseq + 1] == 0x0100)
	{
		// Do nothing if icon isn't animated
		bnriconPalLine = 0;
		bnriconframenumY = 0;
		bannerFlip = GL_FLIP_NONE;
	}
	else
	{
		u16 setframeseq = bnriconframeseq[currentbnriconframeseq];
		bnriconPalLine = SEQ_PAL(setframeseq);
		bnriconframenumY =  SEQ_BMP(setframeseq);
		bool flipH = SEQ_FLIPH(setframeseq);
		bool flipV = SEQ_FLIPV(setframeseq);

		if (flipH && flipV)
		{
			bannerFlip = GL_FLIP_BOTH;
		}
		else if (!flipH && !flipV)
		{
			bannerFlip = GL_FLIP_NONE;
		}
		else if (flipH && !flipV)
		{
			bannerFlip = GL_FLIP_H;
		}
		else if (!flipH && flipV)
		{
			bannerFlip = GL_FLIP_V;
		}

		bannerDelayNum++;
		if (bannerDelayNum >= (setframeseq & 0x00FF))
		{
			bannerDelayNum = 0x0000;
			currentbnriconframeseq++;
			if (bnriconframeseq[currentbnriconframeseq] == 0x0000)
			{
				currentbnriconframeseq = 0; // Reset sequence
			}
		}
	}
}
