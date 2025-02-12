#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <maxmod9.h>

#include "soundbank.h"
//#include "soundbank_bin.h"

#include "common/dsimenusettings.h"
#include "common/gl2d.h"

#include "logo_anniversary.h"
#include "icon_nds.h"
#include "icon_gba.h"
#include "icon_gb.h"
#include "icon_nes.h"
#include "icon_sms.h"
#include "icon_gg.h"
#include "icon_md.h"
#include "icon_snes.h"

extern u16 convertVramColorToGrayscale(u16 val);

static int anniversaryTexID;
static int ndsTexID;
static int gbaTexID;
static int gbTexID;
static int nesTexID;
static int smsTexID;
static int ggTexID;
static int mdTexID;
static int snesTexID;

static glImage anniversaryText[1];
static glImage ndsIcon[1];
static glImage gbaIcon[1];
static glImage gbIcon[(32 / 32) * (64 / 32)];
static glImage nesIcon[1];
static glImage smsIcon[1];
static glImage ggIcon[1];
static glImage mdIcon[1];
static glImage snesIcon[1];

extern u16 bmpImageBuffer[256*192];
extern u16 videoImageBuffer[39][256*144];

extern u16 convertToDsBmp(u16 val);

static char videoFrameFilename[256];

static FILE* videoFrameFile;

extern bool rocketVideo_playVideo;
extern bool rocketVideo_playBackwards;
extern bool rocketVideo_screen;
extern int rocketVideo_videoYpos;
extern int rocketVideo_videoYsize;
extern int rocketVideo_videoFrames;
extern int rocketVideo_videoFps;
extern int rocketVideo_currentFrame;

static int frameDelaySprite = 0;
static bool frameDelaySpriteEven = true;	// For 24FPS
static bool loadFrameSprite = true;

static int anniversaryTextYpos = -14;
static bool anniversaryTextYposMove = false;
static int anniversaryTextYposMoveSpeed = 9;
static int anniversaryTextYposMoveDelay = 0;
static bool anniversaryTextYposMoveDelayEven = true;	// For 24FPS

static int zoomingIconXpos[9] = {-32, 128, 256, 256, 128, -32, -128, 128, 256+96};
static int zoomingIconYpos[9] = {0, -32, -64, 192+64, 192+128, 192, -96, -168, -240};

void twlMenuVideo_loadTopGraphics(void) {
	u16* newPalette;

	// Anniversary
	glDeleteTextures(1, &anniversaryTexID);
	
	newPalette = (u16*)logo_anniversaryPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	anniversaryTexID =
	glLoadTileSet(anniversaryText, // pointer to glImage array
				256, // sprite width
				64, // sprite height
				256, // bitmap width
				64, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Load our 16 color tiles palette
				(u8*) logo_anniversaryBitmap // image data generated by GRIT
				);

	// NDS
	glDeleteTextures(1, &ndsTexID);
	
	newPalette = (u16*)icon_ndsPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	ndsTexID =
	glLoadTileSet(ndsIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap width
				32, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Load our 16 color tiles palette
				(u8*) icon_ndsBitmap // image data generated by GRIT
				);

	// GBA
	glDeleteTextures(1, &gbaTexID);
	
	newPalette = (u16*)icon_gbaPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	gbaTexID =
	glLoadTileSet(gbaIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap width
				32, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Load our 16 color tiles palette
				(u8*) icon_gbaBitmap // image data generated by GRIT
				);

	// GB/GBC
	glDeleteTextures(1, &gbTexID);
	
	newPalette = (u16*)icon_gbPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	gbTexID =
	glLoadTileSet(gbIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				64, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_gbBitmap // Raw image data
				);

	// NES
	glDeleteTextures(1, &nesTexID);

	newPalette = (u16*)icon_nesPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	nesTexID =
	glLoadTileSet(nesIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_nesBitmap // Raw image data
				);

	// SMS
	glDeleteTextures(1, &smsTexID);
	
	newPalette = (u16*)icon_smsPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	smsTexID =
	glLoadTileSet(smsIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_smsBitmap // Raw image data
				);

	// GG
	glDeleteTextures(1, &ggTexID);
	
	newPalette = (u16*)icon_ggPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	ggTexID =
	glLoadTileSet(ggIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_ggBitmap // Raw image data
				);

	// MD
	glDeleteTextures(1, &mdTexID);
	
	newPalette = (u16*)icon_mdPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	mdTexID =
	glLoadTileSet(mdIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_mdBitmap // Raw image data
				);

	// SNES
	glDeleteTextures(1, &snesTexID);
	
	newPalette = (u16*)icon_snesPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(newPalette+i2) = convertVramColorToGrayscale(*(newPalette+i2));
		}
	}
	snesTexID =
	glLoadTileSet(snesIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				(u16*) newPalette, // Image palette
				(u8*) icon_snesBitmap // Raw image data
				);
}

void BootJingleTwlMenu() {
	
	mmInitDefaultMem((mm_addr)0x023A0000);

	mmLoadEffect( SFX_TWLMENUVIDEO );

	mm_sound_effect twlmenuvideosound = {
		{ SFX_TWLMENUVIDEO } ,	// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	mmEffectEx(&twlmenuvideosound);
}

static bool waitTillSoundPlay = true;
static int timeTillSoundPlay = 0;

void twlMenuVideo_topGraphicRender(void) {
	if (ms().colorMode == 1 && waitTillSoundPlay) {
		timeTillSoundPlay++;
		if (timeTillSoundPlay > 60) {
			BootJingleTwlMenu();
			waitTillSoundPlay = false;
		}
	}

	glBegin2D();
	{
		glColor(RGB15(31, 31-(3*ms().blfLevel), 31-(6*ms().blfLevel)));
		glSprite(0, anniversaryTextYpos, GL_FLIP_NONE, anniversaryText);
		if (zoomingIconYpos[0] < 64) {
			glSprite(zoomingIconXpos[0], zoomingIconYpos[0], GL_FLIP_NONE, nesIcon);
		}
		if (zoomingIconYpos[1] < 64) {
			glSprite(zoomingIconXpos[1], zoomingIconYpos[1], GL_FLIP_NONE, smsIcon);
		}
		if (zoomingIconYpos[2] < 64) {
			glSprite(zoomingIconXpos[2], zoomingIconYpos[2], GL_FLIP_NONE, mdIcon);
		}
		if (zoomingIconYpos[3] > 64+32) {
			glSprite(zoomingIconXpos[3], zoomingIconYpos[3], GL_FLIP_NONE, gbIcon);
		}
		if (zoomingIconYpos[4] > 64+32) {
			glSprite(zoomingIconXpos[4], zoomingIconYpos[4], GL_FLIP_NONE, ggIcon);
		}
		if (zoomingIconYpos[5] > 64+32) {
			glSprite(zoomingIconXpos[5], zoomingIconYpos[5], GL_FLIP_NONE, &gbIcon[1]);
		}
		if (zoomingIconYpos[6] < 64) {
			glSprite(zoomingIconXpos[6], zoomingIconYpos[6], GL_FLIP_NONE, snesIcon);
		}
		if (zoomingIconYpos[7] < 64) {
			glSprite(zoomingIconXpos[7], zoomingIconYpos[7], GL_FLIP_NONE, gbaIcon);
		}
		if (zoomingIconYpos[8] < 64) {
			glSprite(zoomingIconXpos[8], zoomingIconYpos[8], GL_FLIP_NONE, ndsIcon);
		}

		glBoxFilled(0, 0, 256, 23, RGB15(0, 0, 0));
		glBoxFilled(0, 168, 256, 192, RGB15(0, 0, 0));
	}
	glEnd2D();
	GFX_FLUSH = 0;

	if (!loadFrameSprite) {
		frameDelaySprite++;
		loadFrameSprite = (frameDelaySprite == 2+frameDelaySpriteEven);
	}

	if (loadFrameSprite) {
		zoomingIconXpos[0] += 4;
		zoomingIconYpos[0] += 4;

		zoomingIconYpos[1] += 4;

		zoomingIconXpos[2] -= 4;
		zoomingIconYpos[2] += 4;

		zoomingIconXpos[3] -= 4;
		zoomingIconYpos[3] -= 4;

		zoomingIconYpos[4] -= 5;

		zoomingIconXpos[5] += 4;
		zoomingIconYpos[5] -= 4;

		zoomingIconXpos[6] += 5;
		zoomingIconYpos[6] += 4;

		zoomingIconYpos[7] += 5;

		zoomingIconXpos[8] -= 5;
		zoomingIconYpos[8] += 6;

		frameDelaySprite = 0;
		frameDelaySpriteEven = !frameDelaySpriteEven;
		loadFrameSprite = false;
	}
	
	if (rocketVideo_playVideo && rocketVideo_currentFrame >= 13) {
		if (!anniversaryTextYposMove) {
			anniversaryTextYposMoveDelay++;
			anniversaryTextYposMove = (anniversaryTextYposMoveDelay == 2+anniversaryTextYposMoveDelayEven);
		}

		if (anniversaryTextYposMove && anniversaryTextYpos < 40) {
			anniversaryTextYpos += anniversaryTextYposMoveSpeed;
			anniversaryTextYposMoveSpeed--;
			if (anniversaryTextYposMoveSpeed < 1) anniversaryTextYposMoveSpeed = 1;

			anniversaryTextYposMoveDelay = 0;
			anniversaryTextYposMoveDelayEven = !anniversaryTextYposMoveDelayEven;
			anniversaryTextYposMove = false;
		}
	}
}

void twlMenuVideo(void) {
	extern bool twlMenuSplash;
	twlMenuSplash = true;
	//dmaFillHalfWords(0, BG_GFX, 0x18000);

	if (ms().colorMode == 0) {
		BootJingleTwlMenu();
	}

	for (int selectedFrame = 0; selectedFrame < 39; selectedFrame++) {
		if (selectedFrame < 10) {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/twlmenupp/frame0%i.bmp", selectedFrame);
		} else {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/twlmenupp/frame%i.bmp", selectedFrame);
		}
		videoFrameFile = fopen(videoFrameFilename, "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x12000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 143;
			for (int i=0; i<256*144; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				videoImageBuffer[selectedFrame][y*256+x] = convertToDsBmp(val);
				x++;
			}
		}
		fclose(videoFrameFile);

		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
	}

	while (zoomingIconYpos[8] < 64) {
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
		swiWaitForVBlank();
	}

	rocketVideo_videoFrames = 43;
	rocketVideo_videoFps = 24;
	rocketVideo_currentFrame = -1;
	rocketVideo_videoYpos = 24;
	rocketVideo_videoYsize = 144;
	rocketVideo_screen = true;
	rocketVideo_playBackwards = false;
	rocketVideo_playVideo = true;

	while (rocketVideo_playVideo && rocketVideo_currentFrame < 5) {
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
		swiWaitForVBlank();
	}

	for (int selectedFrame = 39; selectedFrame <= 43; selectedFrame++) {
		snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/twlmenupp/frame%i.bmp", selectedFrame);
		videoFrameFile = fopen(videoFrameFilename, "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 2, 0x14000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 143;
			for (int i=0; i<256*144; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				videoImageBuffer[selectedFrame-39][y*256+x] = convertToDsBmp(val);
				x++;
			}
			//dmaCopy((void*)videoImageBuffer[0], (u16*)BG_GFX+(256*24), 0x12000);
		}
		fclose(videoFrameFile);

		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
	}

	while (rocketVideo_playVideo) {
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
		swiWaitForVBlank();
	}

	// Change TWL letters to user color
	snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/graphics/TWL_%i.bmp", (int)PersonalData->theme);
	videoFrameFile = fopen(videoFrameFilename, "rb");

	if (videoFrameFile) {
		// Start loading
		fseek(videoFrameFile, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
		fseek(videoFrameFile, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x800, videoFrameFile);
		u16* src = bmpImageBuffer;
		int x = 68;
		int y = 69;
		for (int i=0; i<62*14; i++) {
			if (x >= 130) {
				x = 68;
				y--;
			}
			u16 val = *(src++);
			if (val != 0x7C1F) {
				BG_GFX[(y+rocketVideo_videoYpos)*256+x] = convertToDsBmp(val);
			}
			x++;
		}
	}
	fclose(videoFrameFile);
	
	// Load RocketVideo logo
	videoFrameFile = fopen("nitro:/graphics/logo_rocketvideo.bmp", "rb");

	if (videoFrameFile) {
		// Start loading
		fseek(videoFrameFile, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
		fseek(videoFrameFile, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x18000, videoFrameFile);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 192;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX_SUB[y*256+x] = convertToDsBmp(val);
			x++;
		}
	}
	fclose(videoFrameFile);
	
	for (int i = 0; i < 60 * 3; i++)
	{
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT)) return;
		swiWaitForVBlank();
	}
}