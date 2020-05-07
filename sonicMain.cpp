#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <vector>

#include "sonic.h"

//#define DROP_FRAME

const int MAXN = 1e6 + 7;

const int FRAMESIZE = 8000 / (1000 / 20) * 2;

typedef std::pair<int, int> pii;

std::set<pii> jitterLine;

typedef long long ll;

bool isInLine(int pos) {
  for (auto x : jitterLine) {
    if (x.first <= pos && x.second >= pos) {
      return true;
    }
  }
  return false;
}

void dumpLine() {
  for (auto x : jitterLine) {
    printf("begin:%d, len:%d beginTime:%lf\n", x.first, x.second - x.first,
           x.first * 20.0 / 1000);
  }
}

ll calLvl(uint16_t *buffer, int len) {
  ll sum = 0;
  for (int i = 0; i < len; ++i) {
    sum += (buffer[i] * buffer[i]);
  }
  return sum;
}

void spedUp(uint16_t *outBuf, uint16_t *inBuf1, uint16_t *inBuf2) {
  int p = 0;
  for (int i = 0; i < FRAMESIZE; ++i) {
    if (i & 1) {
      outBuf[p++] = inBuf1[i];
    }
  }
  for (int i = 0; i < FRAMESIZE; ++i) {
    if (i & 1) {
      outBuf[p++] = inBuf2[i];
    }
  }
}

const double eps = 1e-6;

int main(int argc, char *argv[]) {
  // sonicStream tempoStream_;
  // tempoStream_ = sonicCreateStream(8000, 1);

  FILE *file1 = fopen("pcm/xinwenlianbo.pcm", "rb");
  FILE *out_file = fopen("outSped.pcm", "wb");

  int processFrameCount = 3000;
  int maxJitterSize = 20;
  int jitterTimes = 20;
  double speedLow = 0.5;
  double speedHigh = 2.0;

  uint8_t *inBuf = new uint8_t[MAXN];

  uint8_t *it = inBuf;

  uint8_t addBuf[MAXN] = {0};

  srand(time(0));

  /* generate jitter */
  for (int i = 0; i < jitterTimes; ++i) {
    int begin = rand() % processFrameCount;
    int len = rand() % maxJitterSize;
    jitterLine.insert(std::make_pair(begin, begin + len));
  }

  /* dump jitter */
#ifdef DROP_FRAME
  printf("this frame will be frop!\n");
#else
  printf("this frame will be set mute!\n");
#endif
  dumpLine();

  /* if p is in jitter range then repalce it into mute frame */
  for (int p = 0; p < processFrameCount; ++p) {
    if (isInLine(p)) {
      it = inBuf;
      for (int i = 0; i < maxJitterSize; ++i) {
        fread(it, 1, FRAMESIZE, file1);
        it += FRAMESIZE;
      }
      sonicChangeShortSpeed((short int *)inBuf, maxJitterSize * FRAMESIZE / 2,
                            speedLow, 1.0f, 1.0f, 1.0f, 0, 8000, 1);
      fwrite(inBuf, 1, (1.0 * maxJitterSize / speedLow + eps) * FRAMESIZE, out_file);

      it = inBuf;
      for (int i = 0; i < maxJitterSize; ++i) {
        fread(it, 1, FRAMESIZE, file1);
        it += FRAMESIZE;
      }
      sonicChangeShortSpeed((short int *)inBuf, maxJitterSize * FRAMESIZE / 2,
                            speedHigh, 1.0f, 1.0f, 1.0f, 0, 8000, 1);
      fwrite(inBuf, 1, (1.0 * maxJitterSize / speedHigh + eps) * FRAMESIZE,
             out_file);
      p += (maxJitterSize * 2);
    } else {
      fread(inBuf, 1, FRAMESIZE, file1);
      fwrite(inBuf, 1, FRAMESIZE, out_file);
    }
  }

  // sonicChangeShortSpeed((short int*)inBuf, processFrameCount * FRAMESIZE / 2,
  // 0.5f, 1.0f, 1.0f, 1.0f, 0, 8000, 1); fwrite(inBuf, 1, processFrameCount *
  // FRAMESIZE, out_file);

  return 0;
}