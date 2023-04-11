#include <iostream>
#include <iomanip>
#include <intrin.h>
#include <time.h>

#pragma intrinsic(__rdtsc)
unsigned __int64 rdtsc() {
	return __rdtsc();
}

using namespace std;

const int N = 4;
int m1[N][N], m2[N][N], m3[N][N];

void FillMatrix() {
	int count = 1;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			m1[i][j] = rand() % 10 + 10;
			m2[i][j] = rand() % 10 + 1;
			count++;
			m3[i][j] = 0;
		}
	}
}

void PrintMatrix(int x) {
	if (x == 1) {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				cout << setw(3) << m1[i][j] << " ";
			}
			cout << endl;
		}
	}
	else if (x == 2) {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				cout << setw(3) << m2[i][j] << " ";
			}
			cout << endl;
		}
	}
	else {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				cout << setw(3) << m3[i][j] << " ";
			}
			cout << endl;
		}
	}
}

void sse() {
	cout << "SSE:"
		<< endl;
	int numToSub;
	cout << endl << "Number to reduce M1 : ";
	cin >> numToSub;

	unsigned __int64 time = 0;
	unsigned __int64 t = rdtsc();
	_asm {
		movss xmm1, numToSub
		shufps xmm1, xmm1, 0
		mov ecx, 4
		mov ebx, 0
	ROW_LOOP:
		movups xmm0, m1[ebx]
		subps xmm0, xmm1
		movups m1[ebx], xmm0
		add ebx, 16
		loop ROW_LOOP
	}
	time += (rdtsc() - t);

	cout << endl << "M1: reduced by " << numToSub << endl;
	PrintMatrix(1);
	int outLoop, inLoop, offset1 = 0;

	t = rdtsc();
	_asm {
		movups xmm4, m2[0]
		movups xmm5, m2[16]
		movups xmm6, m2[32]
		movups xmm7, m2[48]

		shufps xmm0, xmm4, 0
		shufps xmm0, xmm0, 3
		shufps xmm0, xmm5, 00000100b
		shufps xmm0, xmm0, 01011100b

		shufps xmm1, xmm4, 01010101b
		shufps xmm1, xmm1, 3
		shufps xmm1, xmm5, 01010100b
		shufps xmm1, xmm1, 01011100b

		shufps xmm2, xmm4, 10101010b
		shufps xmm2, xmm2, 3
		shufps xmm2, xmm5, 10100100b
		shufps xmm2, xmm2, 01011100b

		shufps xmm3, xmm4, 11111111b
		shufps xmm3, xmm3, 3
		shufps xmm3, xmm5, 11110100b
		shufps xmm3, xmm3, 01011100b

		shufps xmm4, xmm6, 01001110b
		shufps xmm4, xmm4, 00001110b
		shufps xmm4, xmm7, 01001000b
		shufps xmm4, xmm4, 01011000b

		shufps xmm5, xmm6, 01010100b
		shufps xmm5, xmm5, 00001110b
		shufps xmm5, xmm7, 01010100b
		shufps xmm5, xmm5, 01011000b

		shufps xmm0, xmm4, 01000100b
		shufps xmm1, xmm5, 01000100b

		shufps xmm6, xmm6, 11111110b
		shufps xmm7, xmm7, 11111110b

		shufps xmm6, xmm7, 00000100b
		shufps xmm7, xmm6, 01010100b
		shufps xmm6, xmm6, 01011100b
		shufps xmm7, xmm7, 00000111b

		shufps xmm6, xmm6, 01000000b
		shufps xmm7, xmm7, 01000000b

		shufps xmm2, xmm6, 11010100b
		shufps xmm3, xmm7, 11010100b

		movups m2[0], xmm0
		movups m2[16], xmm1
		movups m2[32], xmm2
		movups m2[48], xmm3

		mov inLoop, 4
		mov outLoop, 4
		mov ebx, 0

	OUT_LOOP:
		mov edx, 0
		mov eax, offset1
		movups xmm1, m1[eax]
		mov ecx, inLoop
	IN_LOOP :
		movups xmm0, m2[edx]
		pmullw xmm0, xmm1
		haddps xmm0, xmm0
		haddps xmm0, xmm0
		movss m3[ebx], xmm0
		add ebx, 4
		add edx, 16
		loop IN_LOOP
		add offset1, 16
		mov ecx, outLoop
		dec outLoop
		loop OUT_LOOP
	}
	time += (rdtsc() - t);

	cout << endl << "M3 = M1 x M2" << endl;
	PrintMatrix(3);
	int row;

	t = rdtsc();

	_asm {
		mov ecx, 4
		mov eax, 0
	loop1:
		movups xmm0, [m1 + 4 * eax]
		movups xmm1, [m3 + 4 * eax]
		addps xmm0, xmm1
		movups[m3 + 4 * eax], xmm0
		add eax, 4
		loop loop1
	}
	time += (rdtsc() - t);

	cout << endl << "M3: M1 + M2" << endl;
	PrintMatrix(3);
	cout << endl;
	cout << "Time: " << time << " ticks" << endl;
}

void no_sse() {
	cout << "no SSE:" << endl;
	int numToAdd;
	cout << endl << "Number to reduce M1: ";
	cin >> numToAdd;

	unsigned __int64 time = 0;
	unsigned __int64 t = rdtsc();
	_asm {
		lea esi, m1
		mov ecx, 16
		mov eax, numToAdd

	SUB_LOOP :
		sub[esi], eax
		add esi, 4
		loop SUB_LOOP
	}
	time += (rdtsc() - t);

	cout << endl << "M1: reduced by " << numToAdd << endl;
	PrintMatrix(1);
	int offset1 = 0, offset2 = 0, offset3 = 0, temp = 0,
		outLoop, inLoop, four_counter = 0, four_add = 4,
		sixteen_add = 0;

	t = rdtsc();
	_asm {
		mov outLoop, 16
	OUT_LOOP:
		mov inLoop, 4
	IN_LOOP :
		lea esi, m1
		mov edx, offset1
		mov eax, [esi + edx]
		lea esi, m2
		mov edx, offset2
		mov ebx, [esi + edx]
		mul ebx
		add temp, eax
		add offset1, 4
		add offset2, 16
		mov ecx, inLoop
		dec inLoop
		loop IN_LOOP
		lea esi, m3
		mov edx, offset3
		jmp NO_TRANSSHIPMENT
	OUT_LOOP_TRANSSHIPMENT :
		jmp OUT_LOOP
	NO_TRANSSHIPMENT :
		mov ebx, temp
		mov[esi + edx], ebx
		add offset3, 4
		mov offset2, 0
		inc four_counter
		cmp four_counter, 4
		je EQUALS_FOUR
		jmp NOT_EQUALS_FOUR
	EQUALS_FOUR :
		mov four_add, 4
		mov four_counter, 0
		add sixteen_add, 16
		jmp CONTINUE
	NOT_EQUALS_FOUR :
		mov edx, four_add
		add offset2, edx
		mov edx, sixteen_add
		mov offset1, edx
		add four_add, 4
	CONTINUE :
		mov temp, 0
		mov ecx, outLoop
		dec outLoop
		loop OUT_LOOP_TRANSSHIPMENT
	}
	time += (rdtsc() - t);

	cout << endl << "M3 = M1 x M2" << endl;
	PrintMatrix(3);
	int row;

	t = rdtsc();

	_asm {
		fld[m1]
		fld[m3]
		fadd
		fstp[m3]
		fld[m1 + 4]
		fld[m3 + 4]
		fadd
		fstp[m3 + 4]
		fld[m1 + 8]
		fld[m3 + 8]
		fadd
		fstp[m3 + 8]
		fld[m1 + 12]
		fld[m3 + 12]
		fadd
		fstp[m3 + 12]
		fld[m1 + 16]
		fld[m3 + 16]
		fadd
		fstp[m3 + 16]
		fld[m1 + 20]
		fld[m3 + 20]
		fadd
		fstp[m3 + 20]
		fld[m1 + 24]
		fld[m3 + 24]
		fadd
		fstp[m3 + 24]
		fld[m1 + 28]
		fld[m3 + 28]
		fadd
		fstp[m3 + 28]
		fld[m1 + 32]
		fld[m3 + 32]
		fadd
		fstp[m3 + 32]
		fld[m1 + 36]
		fld[m3 + 36]
		fadd
		fstp[m3 + 36]
		fld[m1 + 40]
		fld[m3 + 40]
		fadd
		fstp[m3 + 40]
		fld[m1 + 44]
		fld[m3 + 44]
		fadd
		fstp[m3 + 44]
		fld[m1 + 48]
		fld[m3 + 48]
		fadd
		fstp[m3 + 48]
		fld[m1 + 52]
		fld[m3 + 52]
		fadd
		fstp[m3 + 52]
		fld[m1 + 56]
		fld[m3 + 56]
		fadd
		fstp[m3 + 56]
		fld[m1 + 60]
		fld[m3 + 60]
		fadd
		fstp[m3 + 60]
	}
	time += (rdtsc() - t);

	cout << endl << "M3: M1 + M2" << endl;
	PrintMatrix(3);
	cout << endl;
	cout << "Time: " << time << " ticks" << endl;
}

int main() {
	srand(time(NULL));
	FillMatrix();
	cout << "Original matrices:" << endl << endl;
	cout << "M1:" << endl;
	PrintMatrix(1);
	cout << endl;
	cout << "M2:" << endl;
	PrintMatrix(2);
	cout << endl;
	sse();
	cout << "--------------------------" << endl;
	FillMatrix();
	cout << "Original matrices:" << endl << endl;
	cout << "M1:" << endl;
	PrintMatrix(1);
	cout << endl;
	cout << "M2:" << endl;
	PrintMatrix(2);
	cout << endl;
	no_sse();
	cout << endl;
	return 0;
}
