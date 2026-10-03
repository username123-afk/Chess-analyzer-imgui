// vision.h - screen image, board detection, piece recognition. No platform dependencies.
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "chess_core.h"

namespace vision {

struct Image {
    int w = 0, h = 0;
    std::vector<uint8_t> data;  // RGBA8888, row-major
    const uint8_t* at(int x, int y) const { return &data[((size_t)y * w + x) * 4]; }
};

// Parses the output of `screencap` (no -p): [w][h][format](+[colorspace]) + raw pixels.
bool parseRawScreencap(const std::vector<uint8_t>& buf, Image& out, std::string& err);

struct BoardDetect {
    bool found = false;
    float x = 0, y = 0, size = 0;  // board rectangle (square), physical pixels
    int score = 0, maxScore = 0;   // checkerboard agreement
    int ca[3] = {0,0,0}, cb[3] = {0,0,0};  // the two square colours (for fast re-verification)
    int tolUsed = 0;
    std::string why;
};
BoardDetect detectBoard(const Image& im);
// Re-check/track a previously found board on a new frame (tolerant, cheap).
bool verifyBoard(const Image& im, BoardDetect& bd);

constexpr int MG = 40;  // working grid per square
constexpr int TG = 20;  // template grid

struct SqFeat {
    bool empty = true, bad = false;
    float lum = 0;                 // median luminance of piece interior
    float aspect = 1, hfrac = 0;   // bbox aspect and height as fraction of square
    float mask[TG * TG];
    // Diagnostics only (never used for classification): size of the chosen component in working-grid
    // cells, its bounding box, and the estimated square background colour.
    int area = 0, bw = 0, bh = 0;
    float bg[3] = {0, 0, 0};
};
// fgThr: a cell counts as piece foreground when its colour is further than this (sum of |dR|+|dG|+|dB|)
// from the square background. 120 is the normal value; recognize() uses a lower one only as a fallback.
SqFeat analyzeSquare(const Image& im, float x0, float y0, float s, float fgThr = 120.f);

struct Sample { float mask[TG * TG]; float aspect, hfrac; };

struct RecogResult {
    bool ok = false;
    char grid[64];        // screen layout: grid[row*8+col], row 0 = top of screen
    int pieces = 0, unknown = 0;
    std::string unknownSquares;
    float worst = 0;      // worst accepted template distance
    std::string why;
    bool repaired = false;        // one uncertain square was kept from the last stable board
    std::string repairNote;
};

class Recognizer {
public:
    // Learn piece templates from a board currently showing the standard start position.
    bool learnFromStart(const Image& im, float bx, float by, float size, std::string& err);
    RecogResult recognize(const Image& im, float bx, float by, float size) const;
    bool learned() const { return learned_; }
    bool save(const std::string& path) const;
    bool load(const std::string& path);
    float maxDist = 0.25f;  // template acceptance threshold

    // Single-square repair (see recognize()). Never touches maxDist; only runs on the loose retry pass.
    float repairMinDist = 0.30f;  // repair only when maxDist >= this (the 0.32 pass)
    float repairMaxD = 0.85f;     // best template distance above this = garbage, never repaired
    int repairMaxDiff = 8;        // reference is stale if more squares than this differ from it

    // Unknown-square fallback (see recognize()). Re-analyses ONLY squares still unknown after normal
    // recognition, at a lower foreground threshold, on the loose retry pass. Never touches maxDist.
    float fallbackMinDist = 0.30f;  // fallback only runs when maxDist >= this (the 0.32 pass)
    float fallbackFgThr = 80.f;     // foreground threshold for the second analysis (normal is 120)
    float fallbackMargin = 0.10f;   // best same-colour template must beat the 2nd best by at least this
private:
    void resetStable() const;
    void commitStable(const char g[64]) const;
    mutable char ref_[64] = {};   // last stable grid (screen layout)
    mutable char pend_[64] = {};  // candidate awaiting a 2nd identical result
    mutable int pendCount_ = 0;
    mutable bool haveRef_ = false;
    mutable int lastRepairSq_ = -1;
    // log throttling (seconds on a monotonic clock; 0 = never logged) and repairs since the last log line
    mutable double repairLogT_[64] = {};
    mutable int repairSince_[64] = {};
    mutable double diagLogT_[64] = {};

    std::vector<Sample> samples_[12];  // index into "PNBRQKpnbrqk"
    float lumThr_ = 128;
    bool learned_ = false;
};

// grid (screen layout) <-> Board
chess::Board gridToBoard(const char grid[64], bool whiteBottom);
// 1 = white at bottom, 0 = black at bottom, -1 = uncertain
int autoOrientation(const char grid[64], std::string& why);

}  // namespace vision
