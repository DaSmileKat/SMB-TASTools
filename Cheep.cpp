#include <stdio.h>
 
// Thanks to periwinkle who gave me the code for SMB's RNG and cheep spawning mechanism
const int TOTAL_CHEEPS = 1;
int posReq[TOTAL_CHEEPS][16] = { { 2 } }; // Change this to which position(s) you want
int delayReq[TOTAL_CHEEPS][4] = { { 16, 32, 72, 96 } }; // Change this to which delay(s) you want
int speedReq[TOTAL_CHEEPS][16] = { { 1 } }; // Change this to which speed(s) you want
int posReqLen[TOTAL_CHEEPS] = { 1 };
int delayReqLen[TOTAL_CHEEPS] = { 4 };
int speedReqLen[TOTAL_CHEEPS] = { 1 };
int pSpeeds[TOTAL_CHEEPS] =  { 0 }; // 0: speed = 0. 1: speed between 1 and 24, 2: speed above 24 or below 0
int slots[TOTAL_CHEEPS] =    { 3 };
int delay = 0;
													// Gets the byte corresponding to $(7A7+idx)
													// Gets the byte corresponding to $(7A7+idx)
unsigned char getByte(unsigned long long val, int idx) {
	return (val >> ((6 - idx) << 3)) & 0xFF;
}
// Advance by one frame
void advance(unsigned long long& rng) {
	// xor two specific bits of $7A7 and $7A8
	unsigned char a = getByte(rng, 0) & 2, b = getByte(rng, 1) & 2;
	if ((a ^ b) != 0) // If non-zero, shift a 1 into the most-significant bit of $7A7
		rng |= 0x01'00'00'00'00'00'00'00;
	rng >>= 1;
}

int getDelay(unsigned long long rng, int slot) {
	int delays[4] = { 16, 96, 32, 72 };
	rng >>= 40 - slot * 8;
	return delays[rng & 3];
}

int getSpeed(unsigned long long rng, int pSpeed, int slot) {
	rng >>= 48 - slot * 8;
	rng &= 3;
	return pSpeed * 4 + rng;
}

int getPos(unsigned long long rng, int pSpeed, int slot) {
	int index = getSpeed(rng, pSpeed, slot);
	if (!pSpeed) {
		rng >>= 32 - slot * 8;
		if (rng & 768) {
			return rng & 15;
		}
		else {
			return index;
		}
	}
	return index;
}

bool noContain(int* arr, int len, int val) {
	for (int i = 0; i < len; ++i) {
		if (val == arr[i]) {
			return false;
		}
	}
	return true;
}

int main() {
	unsigned long long RNG2 = 0x00A5000000000000; // Starting value
	unsigned long long RNG1 = RNG2;
	for (int i = 0; i < 33031; ++i) {
		advance(RNG2);
	}
	for (int i = 33031; i < 60000; ++i) {
		RNG1 = RNG2;
		for (int cheep = 0; cheep < TOTAL_CHEEPS; ++cheep) {
			int delay = getDelay(RNG2, slots[cheep]);
			int speed = getSpeed(RNG2, pSpeeds[cheep], slots[cheep]);
			int pos = getPos(RNG2, pSpeeds[cheep], slots[cheep]);
		    //printf("%d, %d, %d, %d\n", delay, speed, pos, i + 24);
			if (noContain(delayReq[cheep], delayReqLen[cheep], delay) || noContain(speedReq[cheep], speedReqLen[cheep], speed) || noContain(posReq[cheep], posReqLen[cheep], pos)) {
				break;
			}
			if (cheep == (TOTAL_CHEEPS - 1)) {
				printf("Spawn at: %d\n", i + 24);
				//return;
			}
			for (int j = 0; j < delay; ++j) {
				advance(RNG2);
			}
		}
		RNG2 = RNG1;
		advance(RNG2);
	}
	printf("No solution.");
	return 0;
}
