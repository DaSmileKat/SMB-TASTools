#include <stdio.h>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;
 
// Thanks to periwinkle who gave me the code for SMB's RNG and cheep spawning mechanism
// If you don't see the spawning time for a particular cheep if pausing is allowed, that means the fastest solution is letting it follow the previous cheep naturally without pausing

const int TOTAL_CHEEPS = 4; // Change this to the number of consecutive cheeps you want to brute-force (you must also give the requirements for each cheep)
const int FRAME_START = 33080; // Change this to the frame that you want to start searching from
const int LAG_COUNTER = 24; // Change this to the number of lag frames that has passed so far
const int DO_PAUSE = 1; // 1: Pausing allowed, 0: Pausing not allowed
vector<int> posReq[TOTAL_CHEEPS] = { { 4 }, { 4 }, { 4 }, { 4 } }; // Change this to which position(s) you want
vector<int> delayReq[TOTAL_CHEEPS] = { { 16 }, { 16 }, { 16 }, { 16, 32, 72, 96 } }; // Change this to which delay(s) you want
vector<int> speedReq[TOTAL_CHEEPS] = { { 4 }, { 4 }, { 4 }, { 4 } }; // Change this to which speed(s) you want
int pSpeeds[TOTAL_CHEEPS] =  { 1, 1, 1, 1 }; // 0: speed = 0. 1: speed between 1 and 24, 2: speed above 24 or below 0
int slots[TOTAL_CHEEPS] =    { 3, 2, 1, 0 }; // Change this to the slot that the cheep is spawning in (starting from slot 0)

// DO NOT CHANGE ANYTHING BELOW THIS LINE

int frame = 0;

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
	++frame;
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

int main() {
	unsigned long long RNG2 = 0x00A5000000000000; // Starting value
	unsigned long long RNG1 = RNG2;
	bool noSolution = true;
	for (int i = 0; i < (FRAME_START - LAG_COUNTER); ++i) {
		advance(RNG2);
	}

	if (DO_PAUSE) {
		int fastest = FRAME_START + TOTAL_CHEEPS * 32768;
		int solCheeps[TOTAL_CHEEPS];
		int solFrames[TOTAL_CHEEPS];
		int solCheeps1[TOTAL_CHEEPS];
		int solFrames1[TOTAL_CHEEPS];
		for (int i = 0; i < TOTAL_CHEEPS; ++i) {
			solCheeps1[i] = 0;
			solFrames1[i] = 0;
			solCheeps[i] = 0;
			solFrames[i] = 0;
		}
		for (int pauses = 0; pauses < pow(2, TOTAL_CHEEPS - 1); ++pauses) {
			unsigned long long RNG3 = RNG2;
			frame = 0;
			int frame1 = 0;
			for (int cheep = 0; cheep < TOTAL_CHEEPS; ) {
				int nextPause = cheep;
				for (; !((pauses >> nextPause) % 2) && (nextPause < (TOTAL_CHEEPS - 1)); ++nextPause) {}
				bool done = false;
				if (cheep > 0) {
					frame1 = frame + 45;
					for (int i = 0; i < 45; ++i) {
						advance(RNG2);
					}
				}
				else {
					frame1 = 0;
				}
				for (int i = (FRAME_START - LAG_COUNTER + frame1); i < (FRAME_START - LAG_COUNTER + frame1 + 32768); ++i) {
					RNG1 = RNG2;
					frame = i - FRAME_START + LAG_COUNTER;
					for (int cheep1 = cheep; cheep1 <= nextPause; ++cheep1) {
						int delay = getDelay(RNG2, slots[cheep1]);
						int speed = getSpeed(RNG2, pSpeeds[cheep1], slots[cheep1]);
						int pos = getPos(RNG2, pSpeeds[cheep1], slots[cheep1]);
						//printf("%d, %d, %d, %d\n", delay, speed, pos, i + 24);
						if (find(delayReq[cheep1].begin(), delayReq[cheep1].end(), delay) == delayReq[cheep1].end() ||
							find(posReq[cheep1].begin(), posReq[cheep1].end(), pos) == posReq[cheep1].end() ||
							find(speedReq[cheep1].begin(), speedReq[cheep1].end(), speed) == speedReq[cheep1].end()) {
							break;
						}
						if (cheep1 == nextPause) {
							done = true;
							frame1 = i - FRAME_START + LAG_COUNTER;
							break;
						}
						for (int j = 0; j < delay; ++j) {
							advance(RNG2);
						}
					}
					if (done) {
						break;
					}
					RNG2 = RNG1;
					advance(RNG2);
				}
				if (!done) {
					frame1 = fastest;
					break;
				}
				solCheeps[cheep] = 1;
				solFrames[cheep] = frame1;
				cheep = nextPause + 1;
			}
			if (frame >= fastest) {
				for (int i = 0; i < TOTAL_CHEEPS; ++i) {
					solCheeps[i] = 0;
				}
			}
			else {
				fastest = frame;
				for (int i = 0; i < TOTAL_CHEEPS; ++i) {
					solCheeps1[i] = solCheeps[i];
					solFrames1[i] = solFrames[i] + FRAME_START;
					solCheeps[i] = 0;
				}
			}
			RNG2 = RNG3;
		}
		for (int i = 0; i < TOTAL_CHEEPS; ++i) {
			if (solCheeps1[i]) {
				printf("Spawn cheep %d at: %d\n", i + 1, solFrames1[i]);
			}
		}
		return 0;
	}

	for (int i = (FRAME_START - LAG_COUNTER); i < (FRAME_START - LAG_COUNTER + 32768); ++i) {
		RNG1 = RNG2;
		frame = 0;
		for (int cheep = 0; cheep < TOTAL_CHEEPS; ++cheep) {
			int delay = getDelay(RNG2, slots[cheep]);
			int speed = getSpeed(RNG2, pSpeeds[cheep], slots[cheep]);
			int pos = getPos(RNG2, pSpeeds[cheep], slots[cheep]);
			//printf("%d, %d, %d, %d\n", delay, speed, pos, i + 24);
			if (find(delayReq[cheep].begin(), delayReq[cheep].end(), delay) == delayReq[cheep].end() ||
				find(posReq[cheep].begin(), posReq[cheep].end(), pos) == posReq[cheep].end() ||
				find(speedReq[cheep].begin(), speedReq[cheep].end(), speed) == speedReq[cheep].end()) {
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
