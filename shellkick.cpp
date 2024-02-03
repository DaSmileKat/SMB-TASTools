#include <iostream>
#include <vector>

using namespace std;

const int TOTAL_KICKS = 10; // The total number of shell kicks required
const int KICK_TO_STOMP = 6; // Frames between kicking the shell and stomping the shell
const int FIRST_KICK_FRAME = 3; // The first frame after falling/jumping that you can reach the shell horizontally

const int GRAVITY = 0x90; // Falling gravity
const bool DO_JUMP = false; // Whether you jump at the start instead of falling
const int JUMP_GRAVITY = 0x20; // Jumping gravity (can ignore if DO_JUMP is set to false)
const int JUMP_INIT = -0x400; // Initial Y speed when jumping (can ignore if DO_JUMP is set to false)
const int MAX_JUMP_FRAMES = 10; // Maximum jump frames that may be required (can ignore if DO_JUMP is set to false)

// Y positions are represented as pixel * 0x100 + subpixel
const int Y_MIN = 0x5600;
const int Y_MAX = 0x7500; // Mario can kick/stomp the shell when his Y position is greater than or equal to Y_MIN and less than Y_MAX
const int Y_FLOOR = 0x6000; // Y position of the floor

/*
Output format:

If DO_JUMP is set to false:
1st line: Starting Y subpixel (only the first hexadecimal digit)
2nd line: Minimum number of frames between falling and landing for even cointoss (1000000000 if impossible), followed by a 0 (ignore)
3rd line: The frames when you should kick the shell (frame 0 is the frame Mario starts falling, which is also 1 frame before the frame Mario's Y speed starts changing)
4th and 5th lines: Same as 2nd and 3rd lines, but for odd cointoss
This is repeated for each starting Y subpixel from 0 to 15.

If DO_JUMP is set to true:
1st line: Minimum number of frames between jumping and landing for even cointoss (1000000000 if impossible), followed by the required jump frames
2nd line: The frames when you should kick the shell (frame 0 is the frame you press A)
3rd and 4th lines: Same as 1st and 2nd lines, but for odd cointoss
*/

// DO NOT CHANGE ANYTHING BELOW THIS LINE

void advance(int& pos, int& speed, int& frame, bool jump) {
	pos += speed;
	speed += jump ? JUMP_GRAVITY : GRAVITY;
	if (speed >= 0x480)
		speed = 0x400;
	++frame;
}

int jumpFrames = 0;

int minFrames = 1000000000;
int optJumpFrames = 0;
vector<int> delays(TOTAL_KICKS, 0);

//vector<int> seq = { 8, 6, 11, 9, 8, 10, 6, 7, 9, 6 };

int solve(int pos, int speed, int frame, int kicks) {
	if (kicks == TOTAL_KICKS) {
		while (pos < Y_FLOOR || speed < 0) {
			advance(pos, speed, frame, false);
			if (pos >= Y_FLOOR + 0x500 && speed >= 0)
				return 1000000001;
		}
		return frame;
	}
	int totalRes = 1000000001;
	vector<bool> canKickFrame(50);
	for (int i = 0; ; ++i) {
		canKickFrame[i] = (pos < Y_MAX && pos >= Y_MIN);
		//cout << frame << " " << hex << pos << " " << speed << dec << endl;
		if ((i >= 3 + KICK_TO_STOMP / 2 || (kicks == 0 && frame >= FIRST_KICK_FRAME + KICK_TO_STOMP)) && canKickFrame[i - KICK_TO_STOMP / 2] && canKickFrame[i] && speed >= 0x100) {
			int res = solve(pos, -0x400 + speed % 0x100, frame, kicks + 1);
			totalRes = min(totalRes, res);
			if (res <= minFrames) {
				minFrames = res;
				optJumpFrames = jumpFrames;
				delays[kicks] = i * 2;
			}
		}
		advance(pos, speed, frame, frame < jumpFrames);
		advance(pos, speed, frame, frame < jumpFrames);
		if (pos >= Y_MAX && speed > 0)
			break;
	}
	return totalRes;
}

void output(int cointoss) {
	cout << minFrames << " " << optJumpFrames << endl;
	int s = cointoss;
	for (int i = 0; i < TOTAL_KICKS; ++i) {
		s += delays[i];
		cout << (s - KICK_TO_STOMP) << " ";
	}
	cout << endl;
	minFrames = 1000000000;
}

int main() {
	if (DO_JUMP) {
		for (jumpFrames = 1; jumpFrames <= MAX_JUMP_FRAMES; ++jumpFrames)
			solve(Y_FLOOR, JUMP_INIT, 0, 0);
		output(0);
		for (jumpFrames = 1; jumpFrames <= MAX_JUMP_FRAMES; ++jumpFrames)
			solve(Y_FLOOR + JUMP_INIT, JUMP_INIT + JUMP_GRAVITY, 1, 0);
		output(1);
	}
	else {
		for (int subpix = 0; subpix < 16; ++subpix) {
			solve(Y_FLOOR + 16 * subpix, 0, 0, 0);
			cout << subpix << endl;
			output(0);
			solve(Y_FLOOR + 16 * subpix, GRAVITY, 1, 0);
			output(1);
			cout << endl;
		}
	}
	return 0;
}
