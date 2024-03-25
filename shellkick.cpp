#include <iostream>
#include <vector>

using namespace std;

const int TOTAL_KICKS = 10; // The total number of shell kicks required
vector<int> KICK_TO_STOMP = { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 }; // Frames between kicking the shell and stomping the shell
const int FIRST_KICK_FRAME = 0; // The first frame after falling/jumping that you can reach the shell horizontally

const int GRAVITY = 0x90; // Falling gravity
const bool DO_JUMP = false; // Whether you jump at the start instead of falling
const int JUMP_GRAVITY = 0x28; // Jumping gravity (can ignore if DO_JUMP is set to false)
const int JUMP_INIT = -0x500; // Initial Y speed when jumping (can ignore if DO_JUMP is set to false)
const int MAX_JUMP_FRAMES = 10; // Maximum jump frames that may be required (can ignore if DO_JUMP is set to false)

// Y positions are represented as pixel * 0x100 + subpixel
const int Y_MIN = 0x6100;
const int Y_MAX = 0x8600; // Mario can kick/stomp the shell when his Y position is greater than or equal to Y_MIN and less than Y_MAX
const int Y_FLOOR = 0x7000; // Y position of the floor

/*
Output format (outdated):

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

void advance(int& pos, int& speed, int& frame, bool jump) {
	pos += speed;
	speed += jump ? JUMP_GRAVITY : GRAVITY;
	if ((speed >= 0x480 && speed < 0x500) || speed >= 0x580)
		speed = 0x400;
	++frame;
}

int jumpFrames = 0;

double maxPPF = 0;
int optJumpFrames = 0;
int optFrame = 0;
int optSac = 0;
vector<int> frames(TOTAL_KICKS, 0);
vector<int> curFrames(TOTAL_KICKS, 0);

int cnt = 0;
const vector<int> points = { 100, 200, 400, 500, 800, 1000, 2000, 4000, 5000 };

//vector<int> seq = { 8, 6, 11, 9, 8, 10, 6, 7, 9, 6 };

double solve(int pos, int speed, int frame, int kicks, int sac) {
	if (kicks == TOTAL_KICKS) {
		/*if (sac >= 800)
			return 0;*/
		//int i = 0;
		/*if (sac > 0)
			return 0;*/
		//double res = (double)1 / (double)frame;
		int frame0 = frame;
		int a = 1000000000;
		int p = frame % 2;
		while (pos < Y_FLOOR || speed < 0) {
			advance(pos, speed, frame, false);
			/*++i;
			if (i == 4 && (pos >= Y_MAX || pos < Y_MIN))
				return 1000000001;*/
			++a;
			if (pos < Y_MIN && frame % 2 == p)
				a = 0;
			if (pos >= Y_FLOOR + 0x500 && speed >= 0)
				return 0;
		}
		//if (a > 5)
			//return 0;
		/*if (frame - frame0 > 13 || sac > 0)
			return 0;
		if (frame0 <= 127) {
			cout << frame << " " << jumpFrames << " " << ((pos % 0x100) / 16) << endl;
			for (int i = 0; i < TOTAL_KICKS; ++i)
				cout << curFrames[i] << " ";
			cout << endl;
			++cnt;
		}*/
		double res = /*(double)(43300 - sac) / (double)((max(curFrames[TOTAL_KICKS - 1] - curFrames[0] + 18, frame + 1) + 1) / 2 * 2)*//*(double)1 / (double)((max(curFrames[TOTAL_KICKS - 1] - curFrames[0] + 18, frame + 1) + 1) / 2 * 2);*/(double)(43300 - sac) / (double)(frame);
		/*if ((double)(43700 - sac) / (double)(frame + 5) > 313)
			++cnt;
		return (double)(43700 - sac) / (double)(frame + 5);*/
		//double res = (double)(43300 - sac) / (double)((frame + 1) / 2 * 2);
		if (res > 322) {
			cout << frame << " " << sac << " " << (pos % 0x100) / 16 << endl;
			for (int i = 0; i < TOTAL_KICKS; ++i)
				cout << curFrames[i] << " ";
			cout << endl;
			++cnt;
		}
		if (res >= maxPPF) {
			maxPPF = res;
			optJumpFrames = jumpFrames;
			optFrame = frame;
			optSac = sac;
		}
		return res;
	}
	double totalRes = 0;
	vector<bool> canKickFrame(50);
	int a = (kicks == 9 ? 1000000000 : 0);
	for (int i = 0; ; ++i) {
		canKickFrame[i] = (pos < Y_MAX && pos >= Y_MIN);
		if (pos < Y_MIN)
			a = i;
		//cout << frame << " " << hex << pos << " " << speed << dec << endl;
		if (((kicks != 0 && i >= 2 + KICK_TO_STOMP[kicks] / 2) || (kicks == 0 && frame >= FIRST_KICK_FRAME + KICK_TO_STOMP[kicks])) && canKickFrame[i - KICK_TO_STOMP[kicks] / 2] && canKickFrame[i] && speed >= 0x100 && (kicks != 0 || frame <= 7)/* && (kicks != 1 || frame == 22) && (kicks != 9 || i - KICK_TO_STOMP[kicks] / 2 >= a + 1) && (kicks != 1 || frame == 23) && (kicks != 9 || i == 7) && (kicks != 9 || pos < Y_FLOOR)*/) {
			curFrames[kicks] = frame;
			double res = solve(pos, -0x400 + speed % 0x100, frame, kicks + 1, sac + ((kicks != 0 && i == 2 + KICK_TO_STOMP[kicks] / 2) ? points[kicks - 1] : 0));
			totalRes = max(totalRes, res);
			if (res >= maxPPF) {
				maxPPF = res;
				optJumpFrames = jumpFrames;
				frames[kicks] = frame;
			}
		}
		advance(pos, speed, frame, frame < jumpFrames);
		advance(pos, speed, frame, frame < jumpFrames);
		if (pos >= Y_MAX && speed > 0)
			break;
	}
	return totalRes;
}

void output() {
	cout << maxPPF << " " << optFrame << " " << optSac << " " << optJumpFrames << endl;
	for (int i = 0; i < TOTAL_KICKS; ++i)
		cout << frames[i] << " ";
	cout << endl;
	maxPPF = 0;
}

int main() {
	if (DO_JUMP) {
		for (jumpFrames = 1; jumpFrames <= MAX_JUMP_FRAMES; ++jumpFrames)
			solve(Y_FLOOR + 0x2000, JUMP_INIT, 0, 0, 0);
		//output();
		maxPPF = 0;
		for (jumpFrames = 1; jumpFrames <= MAX_JUMP_FRAMES; ++jumpFrames)
			solve(Y_FLOOR + 0x2000 + JUMP_INIT, JUMP_INIT + JUMP_GRAVITY, 1, 0, 0);
		//output();
		maxPPF = 0;
		/*for (int comb = 0; comb < (1 << (TOTAL_KICKS - 1)); ++comb) {
			for (int i = 1; i < TOTAL_KICKS; ++i)
				KICK_TO_STOMP[i] = ((comb >> (i - 1)) % 2) ? 4 : 10;
			for (jumpFrames = 1; jumpFrames <= MAX_JUMP_FRAMES; ++jumpFrames)
				solve(Y_FLOOR, JUMP_INIT, 0, 0);
			for (jumpFrames = 1; jumpFrames <= MAX_JUMP_FRAMES; ++jumpFrames)
				solve(Y_FLOOR + JUMP_INIT, JUMP_INIT + JUMP_GRAVITY, 1, 0);
		}*/
	}
	else {
		for (int subpix = 0; subpix < 16; ++subpix) {
			cout << subpix << endl;
			solve(0x9000 + 16 * subpix, 0, 0, 0, 0);
			//output();
			maxPPF = 0;
			solve(0x9000 + 16 * subpix, GRAVITY, 1, 0, 0);
			//output();
			maxPPF = 0;
			cout << endl;
		}
		solve(0x6438, 0x400, 0, 0, 0);
		//output();
		solve(0x6838, 0x400, 1, 0, 0);
		//output();
		/*for (int comb = 0; comb < (1 << (TOTAL_KICKS - 1)); ++comb) {
			for (int i = 1; i < TOTAL_KICKS; ++i)
				KICK_TO_STOMP[i] = ((comb >> (i - 1)) % 2) ? 4 : 12;
			for (int subpix = 0; subpix < 16; ++subpix) {
				solve(0x9000 + 16 * subpix, 0, 0, 0);
				minFrames = 1000000000;
				solve(0x9000 + 16 * subpix, GRAVITY, 1, 0);
				minFrames = 1000000000;
			}
		}*/
	}
	cout << cnt << endl;
	return 0;
}
