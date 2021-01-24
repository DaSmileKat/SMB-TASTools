#include <stdio.h>
 
// Thanks to periwinkle who gave me the code for SMB's RNG and cheep spawning mechanism
const int TOTAL_CHEEPS = 1; // Change this to the number of consecutive cheeps you want to brute-force (you must also give the requirements for each cheep)
const int FRAME_START = 33080; // Change this to the frame that you want to start searching from
const int LAG_COUNTER = 24; // Change this to the number of lag frames that has passed so far
int posReq[TOTAL_CHEEPS][16] = { { 8 } }; // Change this to which position(s) you want
int delayReq[TOTAL_CHEEPS][4] = { { 16 } }; // Change this to which delay(s) you want
int speedReq[TOTAL_CHEEPS][16] = { { 8 } }; // Change this to which speed(s) you want
int posReqLen[TOTAL_CHEEPS] = { 1 }; // Change this to how many valid positions there are
int delayReqLen[TOTAL_CHEEPS] = { 1 }; // Change this to how many valid delays there are
int speedReqLen[TOTAL_CHEEPS] = { 1 }; // Change this to how many valid speeds there are
int pSpeeds[TOTAL_CHEEPS] =  { 2 }; // 0: speed = 0. 1: speed between 1 and 24, 2: speed above 24 or below 0
int slots[TOTAL_CHEEPS] =    { 3 }; // Change this to the slot that the cheep is spawning in (starting from slot 0)

// DO NOT CHANGE ANYTHING BELOW THIS LINE

int delay = 0;

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
	bool noSolution = true;
	for (int i = 0; i < (FRAME_START - LAG_COUNTER); ++i) {
		advance(RNG2);
	}
	for (int i = (FRAME_START - LAG_COUNTER); i < (FRAME_START - LAG_COUNTER + 32768); ++i) {
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
				printf("Spawn at: %d\n", i + LAG_COUNTER);
				noSolution = false;
				//return;
			}
			for (int j = 0; j < delay; ++j) {
				advance(RNG2);
			}
		}
		RNG2 = RNG1;
		advance(RNG2);
	}
	if (noSolution) {
		printf("No solution.");
	}
	return 0;
}
