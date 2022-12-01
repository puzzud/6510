/*
 *  MOS6569.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 06/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MOS6569.h"
#include "Util.h"



static char CMOS6569TextMap[128] = 
				 {    '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
					  'P','Q','R','S','T','U','V','W','X','Y','Z','[','x',']','^','x',
                      ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
                      '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
                      'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
                      'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
                      ' ','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
                      'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x' };

CMOS6569::CMOS6569(){
	memset(mRegs, 0, sizeof(mRegs));
	memset(mColorRam, 0, sizeof(mColorRam));

	irq = false;

	rasterLine = 0;
	perLineClockCycle = 0;

	rasterLineCompare = 0;

	mBus = CBus::GetInstance(); 
	mBus->Register(eBusVic, this, 0xD000, 0xD3FF);

	mRenderer = NULL;
}

void CMOS6569::SetChar(u16 address, u8 c){
	cout << "???????????????" << CMOS6569TextMap[c] << flush;// << endl;
}

u8 CMOS6569::GetDeviceID(){
	return eBusVic;
}

void CMOS6569::Cycle(){
	if (++perLineClockCycle == NTSC_FIELD_CYCLES_PER_LINE){
		perLineClockCycle = 0;

		RenderFieldLinePixelColorBuffers(); // Must be called before TestSpriteCollision.
		TestSpriteCollision();

		if (mRenderer != NULL){
			mRenderer->OnRasterLineCompleted((unsigned int)rasterLine);
		}

		if (++rasterLine == NTSC_FIELD_LINE_HEIGHT + 1){
			rasterLine = 0;
		}
		
		if (rasterLine == rasterLineCompare){
			mRegs[0xD019 - 0xD000] |= 0x81; // Mark possible interrupt flags.
		}
	}

	bool rasterLineCompareIrqEnabled = (mRegs[0xD01A - 0xD000] & 0x01) != 0;
	bool rasterLineCompareMatched = (mRegs[0xD019 - 0xD000] & 0x01) != 0;
	irq = rasterLineCompareIrqEnabled && rasterLineCompareMatched;
}

u8 CMOS6569::Peek(u16 address){
	if(address >= 0xD800 && address <= 0xDBFF){
		// NOTE: On a real system the high nibble
		// of each byte is unreliable and essentially
		// random noise from recent bus operations.
		// Random data could be inserted on peek,
		// but for now it is AND'd out.
		return mColorRam[address-0xD800] & 0x0F;
	}

	// TODO: Implement mirroring.
	if(address >= 0xD000 && address <= (0xD000 + 47 - 1)){
		if(address == 0xD011){
			u8 val = mRegs[address-0xD000];

			// Bit 7 is different when read,
			// and is based on current position
			// of raster line;
			val &= 0x7F;
			val |= (rasterLine >> 1) & 0x80;
			return val;
		}else if (address == 0xD012){
			return (u8)(rasterLine & 0xFF);
		}else if(address == 0xD01E){
			u8 val = mRegs[address-0xD000];
			mRegs[address-0xD000] = 0;
			return val;
		}else if(address == 0xD01F){
			u8 val = mRegs[address-0xD000];
			mRegs[address-0xD000] = 0;
			return val;
		}

		return mRegs[address - 0xD000];
	}
	
	return 0xff;
}


int CMOS6569::Poke(u16 address, u8 val){
	if (address >= 0xD800 && address <= 0xDBFF){
		mColorRam[address - 0xD800] = val;
	}else if(address >= 0xD000 && address <= (0xD000 + 47 - 1)){
		// TODO: Implement mirroring.

		if (address == 0xD011){
			rasterLineCompare = (rasterLineCompare & 0xFF) | ((val & 0x80) << 1);
		}else if (address == 0xD012){
			rasterLineCompare = (rasterLineCompare & 0x100) | val;
		}else if (address == 0xD019){
			u8 previousVal = mRegs[address - 0xD000];
			val = previousVal & ~(val & 0x0F);
			val &= 0x7F;
			if ((val & 0x0F) != 0){
				val |= 0x80;
			}
		}else if(address == 0xD01E){
			return 0; // Can't poke into this register.
		}else if(address == 0xD01F){
			return 0; // Can't poke into this register.
		}

		mRegs[address - 0xD000] = val;
	}

	return 0;
}


bool CMOS6569::GetIRQ(){
	return irq;
}


u16 CMOS6569::GetCharacterMemoryOffset(){
	u8 vicCharacterMemoryBankIndex = (chipMemoryControlRegister >> 1) & 0x07;
	return vicCharacterMemoryBankIndex * CHARACTER_MEMORY_BANK_SIZE;
}


u16 CMOS6569::GetScreenMemoryOffset(){
	u8 vicScreenMemoryBankIndex = (chipMemoryControlRegister >> 4) & 0x0F;
	return vicScreenMemoryBankIndex * SCREEN_MEMORY_BANK_SIZE;
}


u16 CMOS6569::GetSpritePointersMemoryOffset(){
	return GetScreenMemoryOffset() + SCREEN_MEMORY_BANK_SIZE - NUMBER_OF_HARDWARE_SPRITES;
}


bool CMOS6569::IsSpriteEnabled(unsigned int spriteIndex){
	u8 spriteEnable = mRegs[0xD015-0xD000];
	return (spriteEnable & (1 << spriteIndex)) != 0;
}


u16 CMOS6569::GetSpriteXPosition(unsigned int spriteIndex){
	u8 spriteXPositionMsbs = mRegs[0xD010-0xD000];
	u8 spriteXPositionLsb = mRegs[0xD000-0xD000 + (spriteIndex * 2)];

	u16 spriteXPosition = spriteXPositionLsb;
	if ((spriteXPositionMsbs & (1 << spriteIndex)) != 0){
		spriteXPosition += 0x100;
	}

	return spriteXPosition;
}


u8 CMOS6569::GetSpriteYPosition(unsigned int spriteIndex){
	return mRegs[0xD001-0xD000 + (spriteIndex * 2)];
}


void CMOS6569::SetSpriteXPosition(unsigned int spriteIndex, u16 xPosition){
	u8 spriteXPositionMsbs = mRegs[0xD010-0xD000];
	
	mRegs[0xD010-0xD000] = (xPosition > 255) ?
		spriteXPositionMsbs | (1 << spriteIndex) :
		spriteXPositionMsbs & ~(1 << spriteIndex);

	mRegs[0xD000-0xD000 + (spriteIndex * 2)] = (u8)xPosition & 0xff;
}


void CMOS6569::SetSpriteYPosition(unsigned int spriteIndex, u8 yPosition){
	mRegs[0xD001-0xD000 + (spriteIndex * 2)] = yPosition;
}


unsigned int CMOS6569::GetSpriteHorizontalScale(unsigned int spriteIndex){
	u8 spriteXExpands = mRegs[0xD01D-0xD000];
	return ((spriteXExpands & (1 << spriteIndex)) != 0) ? 2 : 1;
}


unsigned int CMOS6569::GetSpriteVerticalScale(unsigned int spriteIndex){
	u8 spriteYExpands = mRegs[0xD017-0xD000];
	return ((spriteYExpands & (1 << spriteIndex)) != 0) ? 2 : 1;
}


u8 CMOS6569::GetSpriteWidth(unsigned int spriteIndex){
	return HARDWARE_SPRITE_WIDTH * GetSpriteHorizontalScale(spriteIndex);
}


u8 CMOS6569::GetSpriteHeight(unsigned int spriteIndex){
	return HARDWARE_SPRITE_HEIGHT * GetSpriteVerticalScale(spriteIndex);
}


bool CMOS6569::IsSpriteOnLine(unsigned int spriteIndex, unsigned int lineNumber){
	u8 spriteYPosition = GetSpriteYPosition(spriteIndex);
	return lineNumber >= spriteYPosition && lineNumber < (spriteYPosition + GetSpriteHeight(spriteIndex));
}


u8 CMOS6569::GetSpritePointerValue(unsigned int spriteIndex){
	u16 address = mBus->GetVicMemoryBankStartAddress() + GetSpritePointersMemoryOffset() + spriteIndex;

	mBus->SetMode(eBusModeVic);
	return mBus->Peek(address);
}


u16 CMOS6569::GetSpriteDataMemoryOffset(unsigned int spriteIndex){
	return GetSpritePointerValue(spriteIndex) * HARDWARE_SPRITE_DATA_BLOCK_SIZE;
}


u8 CMOS6569::GetSpriteColor(unsigned int spriteIndex){
	return mRegs[0xD027-0xD000 + spriteIndex] & 0x0F;
}


bool CMOS6569::IsSpriteMultiColor(unsigned int spriteIndex){
	u8 spriteMultiColor = mRegs[0xD01C-0xD000];
	return (spriteMultiColor & (1 << spriteIndex)) != 0;
}


void CMOS6569::RenderFieldLinePixelColorBuffers(){
	memset(backgroundFieldLinePixelColorBuffer, 0, sizeof(backgroundFieldLinePixelColorBuffer));

	memset(spriteFieldLinePixelColorBuffers, 0,
		sizeof(spriteFieldLinePixelColorBuffers));

	// HARDWARE_SPRITE_TO_SCREEN_X_OFFSET accounts for
	// border and HBlank to match with screen background.
	DrawBackgroundRowToBuffer(rasterLine, eByteRenderModeCharacterCollision,
		backgroundFieldLinePixelColorBuffer + HARDWARE_SPRITE_TO_SCREEN_X_OFFSET);

	for (int spriteIndex = 0; spriteIndex < NUMBER_OF_HARDWARE_SPRITES; ++spriteIndex){
		if (!IsSpriteEnabled(spriteIndex)){
			continue;
		}
		
		if (!IsSpriteOnLine(spriteIndex, rasterLine)){
			continue;
		}

		// Go forward as many sprite lines (3 bytes) to match up to this line based on position.
		u8 spriteRowIndex =
			(rasterLine - GetSpriteYPosition(spriteIndex)) / GetSpriteVerticalScale(spriteIndex);

		u8* spriteFieldLinePixelColorBuffer = (u8*)&spriteFieldLinePixelColorBuffers[spriteIndex];

		DrawSpriteRowToBuffer(
			spriteIndex,
			spriteRowIndex,
			spriteFieldLinePixelColorBuffer + GetSpriteXPosition(spriteIndex));
	}
}


void CMOS6569::TestSpriteCollision(){
	// Check & retain collisions.
	u8 spriteSpriteCollisions = mRegs[0xD01E - 0xD000];
	u8 spriteBackgroundCollisions = mRegs[0xD01F - 0xD000];

	for (int x = 0; x < HARDWARE_SPRITE_PIXEL_BUFFER_SIZE; ++x){
		static u8 spritesPixelsThisX[NUMBER_OF_HARDWARE_SPRITES];
		u8 numberOfSpritesThisX = 0;

		for (int spriteIndex = 0; spriteIndex < NUMBER_OF_HARDWARE_SPRITES; ++spriteIndex){
			u8* spriteFieldLinePixelColorBuffer = (u8*)&spriteFieldLinePixelColorBuffers[spriteIndex];

			u8 pixelColor = spriteFieldLinePixelColorBuffer[x];
			spritesPixelsThisX[spriteIndex] = pixelColor;
			if (pixelColor != 0){
				++numberOfSpritesThisX;
			}
		}

		// Sprite to sprite.
		if (numberOfSpritesThisX > 1){
			u8 spriteSpriteCollisionsThisX = 0;

			for (int spriteIndex = 0; spriteIndex < NUMBER_OF_HARDWARE_SPRITES; ++spriteIndex){
				if (spritesPixelsThisX[spriteIndex] == 0){
					continue;
				}
				
				spriteSpriteCollisionsThisX |= (1 << spriteIndex);
			}

			spriteSpriteCollisions |= spriteSpriteCollisionsThisX;
		}

		// Sprite to background.
		u8 backgroundPixelThisX = backgroundFieldLinePixelColorBuffer[x];

		if ((numberOfSpritesThisX > 0) && (backgroundPixelThisX != 0)){
			u8 spriteBackgroundCollisionsThisX = 0;

			for (int spriteIndex = 0; spriteIndex < NUMBER_OF_HARDWARE_SPRITES; ++spriteIndex){
				if (spritesPixelsThisX[spriteIndex] == 0){
					continue;
				}

				spriteBackgroundCollisionsThisX |= (1 << spriteIndex);
			}

			spriteBackgroundCollisions |= spriteBackgroundCollisionsThisX;
		}
	}

	mRegs[0xD01E - 0xD000] |= spriteSpriteCollisions;
	mRegs[0xD01F - 0xD000] |= spriteBackgroundCollisions;
}


void CMOS6569::RegisterHWScreen(CVICHWScreen* screen){
	// Detach previous one from VIC.
	if (mRenderer != NULL && mRenderer != screen){
		mRenderer->SetVic(NULL);
	}
	
	mRenderer = screen;

	// Attach new one to VIC.
	if (screen != NULL){
		screen->SetVic(this);
	}
}


void CMOS6569::HWNeedsRedraw(){
}


// mode parameter indicates character or sprite mode,
// because of the discrepancy between how bit pairs
// map to color values respectively.
// This method assumes at least 16 bytes of valid memory at
// pixelColorBuffer, in case of horizontal scaling.
void CMOS6569::DrawByteToBuffer(u8 byte, u8* pixelColorBuffer, u8* colorCodes, eByteRenderMode mode, bool multiColor, unsigned int horizontalScale)
{
	// Keep local copy of color codes,
	// some modes may adjust how they are interpreted.
	static u8 adjustedColorCodes[3];
	memcpy(adjustedColorCodes, colorCodes, 3);

	// Determine character multicolor mode behavior based on byte.
	if (mode == eByteRenderModeCharacter || mode == eByteRenderModeCharacterCollision)
	{
		if (multiColor)
		{
			if ((adjustedColorCodes[2] & 0x08) == 0)
			{
				// Fall back to single color if color code
				// has bit 4 set.
				multiColor = false;
			}

			// This color can only be in range 0-7,
			// in multicolor character mode.
			adjustedColorCodes[2] &= 0x07;
		}
	}

	// Characters & sprites use different bit pairs.
	// This variable is an attempt to help normalize
	// the order of colors provided by colorCodes.
	u8 singleColorCode =
		(mode == eByteRenderModeSprite) ? adjustedColorCodes[1] : adjustedColorCodes[2];

	unsigned int bufferIndex = (8 * horizontalScale) - 1;

	for (int x = 0; x < 8; x += 2, byte >>= 2)
	{
		u8 bitPair = byte & 0x03;

		if (bitPair == 0 ||
			(mode == eByteRenderModeCharacterCollision && bitPair == 0x01)) // Bit pair 01 with multicolor character.
		{
			bufferIndex -= 2 * horizontalScale;
			continue;
		}
		
		for (int bitIndex = 0; bitIndex < 2; ++bitIndex)
		{
			if (!multiColor)
			{
				if ((bitPair & (1 << bitIndex)) == 0)
				{
					bufferIndex -= horizontalScale;
					continue;
				}
			}

			for (unsigned int s = 0; s < horizontalScale; ++s, --bufferIndex)
			{
				// Plus 1 to account for using 0 for transparency.
				// Needs to be readjusted on consumption.
				pixelColorBuffer[bufferIndex] =
					(multiColor ? adjustedColorCodes[bitPair - 1] : singleColorCode) + 1;
			}
		}
	}
}


void CMOS6569::DrawBackgroundRowToBuffer(unsigned int fieldLineNumber, eByteRenderMode mode, u8* pixelColorBuffer){
	u16 vicMemoryBankStartAddress = mBus->GetVicMemoryBankStartAddress();
	u16 vicScreenMemoryStartAddress = vicMemoryBankStartAddress + GetScreenMemoryOffset();
	u16 vicCharacterMemoryStartAddress = vicMemoryBankStartAddress + GetCharacterMemoryOffset();
	
	static u8 colorCodes[3];
	bool multiColorCharacters = (mRegs[0xD016-0xD000] & 0x10) != 0;
	if (multiColorCharacters)
	{
		colorCodes[0] = mRegs[0xD022-0xD000] & 0x0F;
		colorCodes[1] = mRegs[0xD023-0xD000] & 0x0F;
	}

	int screenLineNumber = fieldLineNumber - HARDWARE_SPRITE_TO_SCREEN_Y_OFFSET;
	unsigned int rowIndex = screenLineNumber / CHARACTER_HEIGHT;
	unsigned int characterRowIndex = screenLineNumber % CHARACTER_HEIGHT;

	// 1 column (1 byte) per cycle.
	for (unsigned int columnIndex = 0; columnIndex < SCREEN_CHAR_WIDTH; ++columnIndex)
	{
		int characterScreenMemoryOffset = (rowIndex * SCREEN_CHAR_WIDTH) + columnIndex;

		colorCodes[2] = mBus->PeekDevice(eBusVic, 0xD800 + characterScreenMemoryOffset) & 0x0F;

		unsigned char shapeCode = mBus->PeekDevice(eBusRam, vicScreenMemoryStartAddress + characterScreenMemoryOffset);
		int characterRomCharOffset = shapeCode * CHARACTER_HEIGHT;
		
		mBus->SetMode(eBusModeVic);
		unsigned char charRowByte = mBus->Peek(vicCharacterMemoryStartAddress + characterRomCharOffset + characterRowIndex);

		DrawByteToBuffer(
			charRowByte,
			pixelColorBuffer,
			colorCodes,
			mode,
			multiColorCharacters);
		
		pixelColorBuffer += 8; // NOTE: Mutates function argument.
	}
}


void CMOS6569::DrawSpriteRowToBuffer(unsigned int spriteIndex, unsigned int rowIndex, u8* pixelColorBuffer){
	u16 vicMemoryBankStartAddress = mBus->GetVicMemoryBankStartAddress();
	
	static u8 colorCodes[3];
	colorCodes[0] = mRegs[0xD025-0xD000] & 0x0F;
	colorCodes[1] = GetSpriteColor(spriteIndex);
	colorCodes[2] = mRegs[0xD026-0xD000] & 0x0F;

	u16 spriteDataAddress = vicMemoryBankStartAddress +
		GetSpriteDataMemoryOffset(spriteIndex);
	
	spriteDataAddress += (rowIndex * 3);

	for (int byteIndex = 0; byteIndex < 3; ++byteIndex)
	{
		mBus->SetMode(eBusModeVic);
		u8 spriteRowByte = mBus->Peek(spriteDataAddress + byteIndex);

		unsigned int spriteHorizontalScale = GetSpriteHorizontalScale(spriteIndex);

		DrawByteToBuffer(
			spriteRowByte,
			pixelColorBuffer,
			colorCodes,
			eByteRenderModeSprite,
			IsSpriteMultiColor(spriteIndex),
			spriteHorizontalScale);
		
		pixelColorBuffer += (spriteHorizontalScale * 8); // NOTE: Mutates function argument.
	}
}


void CMOS6569::RenderGraphicsToBuffer(u8* pixelColorBuffer){
	if ((mRegs[0xD011-0xD000] & 0x10) == 0){
		// Screen is blanked, so bail out.
		return;
	}

	// Fully render background (no multicolor collision provision).
	// HARDWARE_SPRITE_TO_SCREEN_X_OFFSET accounts for
	// border and HBlank to match with screen background.
	static u8 renderBackgroundFieldLinePixelColorBuffer[HARDWARE_SPRITE_PIXEL_BUFFER_SIZE];
	memset(renderBackgroundFieldLinePixelColorBuffer, 0, HARDWARE_SPRITE_PIXEL_BUFFER_SIZE);
	DrawBackgroundRowToBuffer(rasterLine, eByteRenderModeCharacter,
		renderBackgroundFieldLinePixelColorBuffer + HARDWARE_SPRITE_TO_SCREEN_X_OFFSET);
	
	// TODO: Clamp to bounds of these buffers (add method arguments to control offset and length),
	// in order to prevent overflow; also will facilitate if buffer is ever
	// processed in 8 bits at a time per clock.
	u8 spritePriority = mRegs[0xD01B-0xD000];
	static bool spriteGivesBackgroundPriority[NUMBER_OF_HARDWARE_SPRITES];
	for (int spriteIndex = 0; spriteIndex < NUMBER_OF_HARDWARE_SPRITES; ++spriteIndex){
		spriteGivesBackgroundPriority[spriteIndex] = ((1 << spriteIndex) & spritePriority) != 0;
	}

	// Cycle through each pixel.
	for (unsigned int x = 0; x < HARDWARE_SPRITE_PIXEL_BUFFER_SIZE; ++x){
		
		// Cycle through each sprite, ascending.
		for (int spriteIndex = 0; spriteIndex < NUMBER_OF_HARDWARE_SPRITES; ++spriteIndex){
			// Check this sprite's pixel.
			u8 spritePixelColor = spriteFieldLinePixelColorBuffers[spriteIndex][x];
			if (spritePixelColor != 0){
				// Sprite pixel is opaque.

				if (spriteGivesBackgroundPriority[spriteIndex] && backgroundFieldLinePixelColorBuffer[x] != 0){
					// Sprite gives priority to an opaque background graphic;
					// that's what gets rendered.
					// NOTE: Using collision based background field buffer.
					pixelColorBuffer[x] = backgroundFieldLinePixelColorBuffer[x];
				}else{
					// Just draw this sprite's pixel.
					pixelColorBuffer[x] = spritePixelColor;
				}

				// In either case, a pixel has been rendered;
				// go to next pixel.
				break;
			}

			// Sprite pixel is transparent, check next sprite "below it".
			if (spriteIndex == (NUMBER_OF_HARDWARE_SPRITES - 1)){
				// Unless it's the last sprite; in which case,
				// copy over an opaque background graphics pixel.
				// NOTE: Uses fully rendered background graphics.
				if (renderBackgroundFieldLinePixelColorBuffer[x] != 0){
					pixelColorBuffer[x] = renderBackgroundFieldLinePixelColorBuffer[x];
				}
			}
		}
	}
}
