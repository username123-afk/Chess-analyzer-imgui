#include "chess_core.h"
#include <algorithm>
#include <cstring>
namespace chess {
Board::Board() { std::fill(sq, sq + 64, '.'); }
bool Board::operator==(const Board& o) const { return std::memcmp(sq, o.sq, 64) == 0; }
Board startBoard() {
    Board b; const char* r = "RNBQKBNR"; const char* p = "rnbqkbnr";
    for (int f = 0; f < 8; f++) { b.at(f, 0) = r[f]; b.at(f, 1) = 'P'; b.at(f, 6) = 'p'; b.at(f, 7) = p[f]; }
    return b;
}
bool isWhite(char p) { return p >= 'A' && p <= 'Z'; }
bool validate(const Board& b, bool kings, bool, std::string& why) {
    int wk = 0, bk = 0;
    for (char p : b.sq) {
        if (std::string("PNBRQKpnbrqk.").find(p) == std::string::npos) { why = "invalid piece"; return false; }
        if (p == 'K') wk++;
        if (p == 'k') bk++;
    }
    if (kings && (wk != 1 || bk != 1)) {
        why = "kings: white=" + std::to_string(wk) + " black=" + std::to_string(bk) + " (need 1 each)";
        return false;
    }
    return true;
}
static bool attacked(const Board& b, int f, int r, bool byWhite) {
    auto at = [&](int ff, int rr) -> char { return (ff < 0 || ff > 7 || rr < 0 || rr > 7) ? '.' : b.at(ff, rr); };
    char P = byWhite ? 'P' : 'p', N = byWhite ? 'N' : 'n', B = byWhite ? 'B' : 'b';
    char R = byWhite ? 'R' : 'r', Q = byWhite ? 'Q' : 'q', K = byWhite ? 'K' : 'k';
    int pr = byWhite ? r - 1 : r + 1;
    if (at(f - 1, pr) == P || at(f + 1, pr) == P) return true;
    static const int kn[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
    for (auto& k : kn) if (at(f + k[0], r + k[1]) == N) return true;
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++) {
            if (!dx && !dy) continue;
            if (at(f + dx, r + dy) == K) return true;
            bool diag = dx && dy;
            for (int k = 1; k < 8; k++) {
                int ff = f + dx * k, rr = r + dy * k;
                if (ff < 0 || ff > 7 || rr < 0 || rr > 7) break;
                char c = b.at(ff, rr);
                if (c == '.') continue;
                if (c == Q || (diag && c == B) || (!diag && c == R)) return true;
                break;
            }
        }
    return false;
}
bool validateFull(const Board& b, char stm, std::string& why) {
    if (!validate(b, true, false, why)) return false;
    int wp = 0, bp = 0, wn = 0, bn = 0, wkf = 0, wkr = 0, bkf = 0, bkr = 0;
    for (int r = 0; r < 8; r++)
        for (int f = 0; f < 8; f++) {
            char p = b.at(f, r);
            if (p == '.') continue;
            if (isWhite(p)) wn++; else bn++;
            if (p == 'P') wp++;
            if (p == 'p') bp++;
            if ((p == 'P' || p == 'p') && (r == 0 || r == 7)) { why = "pawn on first/last rank"; return false; }
            if (p == 'K') { wkf = f; wkr = r; }
            if (p == 'k') { bkf = f; bkr = r; }
        }
    if (wp > 8 || bp > 8) { why = "too many pawns"; return false; }
    if (wn > 16 || bn > 16) { why = "too many pieces"; return false; }
    if (std::abs(wkf - bkf) <= 1 && std::abs(wkr - bkr) <= 1) { why = "kings adjacent"; return false; }
    bool whiteToMove = stm == 'w';
    // side NOT to move must not be in check
    if (whiteToMove ? attacked(b, bkf, bkr, true) : attacked(b, wkf, wkr, false)) {
        why = std::string(whiteToMove ? "black" : "white") +
              " king is in check but it is not their turn (wrong 'Side to move' or misread piece)";
        return false;
    }
    return true;
}
std::string sanitizeCastling(const Board& b, const std::string& c) {
    std::string out;
    auto has = [&](char ch) { return c.find(ch) != std::string::npos; };
    if (has('K') && b.at(4, 0) == 'K' && b.at(7, 0) == 'R') out += 'K';
    if (has('Q') && b.at(4, 0) == 'K' && b.at(0, 0) == 'R') out += 'Q';
    if (has('k') && b.at(4, 7) == 'k' && b.at(7, 7) == 'r') out += 'k';
    if (has('q') && b.at(4, 7) == 'k' && b.at(0, 7) == 'r') out += 'q';
    return out.empty() ? "-" : out;
}
std::string sanitizeEp(const Board& b, char stm, const std::string& ep) {
    if (ep.empty() || ep[0] < 'a' || ep[0] > 'h') return "-";
    int f = ep[0] - 'a';
    if (stm == 'w') {
        if (b.at(f, 4) == 'p' && b.at(f, 5) == '.' && b.at(f, 6) == '.') return std::string(1, ep[0]) + "6";
    } else {
        if (b.at(f, 3) == 'P' && b.at(f, 2) == '.' && b.at(f, 1) == '.') return std::string(1, ep[0]) + "3";
    }
    return "-";
}
std::string fen(const Board& b, char side, const std::string& castling, const std::string& ep) {
    std::string s;
    for (int r = 7; r >= 0; r--) {
        int n = 0;
        for (int f = 0; f < 8; f++) {
            char p = b.at(f, r);
            if (p == '.') n++;
            else { if (n) { s += char('0' + n); n = 0; } s += p; }
        }
        if (n) s += char('0' + n);
        if (r) s += '/';
    }
    s += ' '; s += side; s += ' '; s += castling; s += ' '; s += ep; s += " 0 1";
    return s;
}
}

namespace chess {

static bool sameKnown(const Board& cand, const Board& partial, const bool known[64]) {
    for (int i = 0; i < 64; ++i)
        if (known[i] && cand.sq[i] != partial.sq[i]) return false;
    return true;
}

static void addCandidate(const Board& b, char stm, const Board& partial, const bool known[64],
                         std::vector<Board>& out) {
    if (!sameKnown(b, partial, known)) return;

    std::string why;
    char next = stm == 'w' ? 'b' : 'w';
    // validateFull() checks that the side that just moved is not left in check
    // and that the opponent (the new side to move) is not already in an
    // impossible "not their turn" check state.
    if (!validateFull(b, next, why)) return;
    out.push_back(b);
}

static void addPromotionMoves(const Board& prev, char stm, const Board& partial, const bool known[64],
                              std::vector<Board>& out, int ff, int fr, int tf, int tr) {
    const bool white = stm == 'w';
    const char* ps = white ? "QRBN" : "qrbn";
    for (int i = 0; i < 4; ++i) {
        Board b = prev;
        b.at(tf, tr) = ps[i];
        b.at(ff, fr) = '.';
        addCandidate(b, stm, partial, known, out);
    }
}

bool recoverOneMove(const Board& prev, char stm, const Board& partial, const bool known[64],
                    Board& out, std::string& why) {
    std::vector<Board> cand;
    const bool white = stm == 'w';

    // The previous position itself must be a structurally/legal position for
    // this side to move. This also prevents recovery from inventing moves out
    // of a corrupt previous state.
    std::string prevWhy;
    if (!validateFull(prev, stm, prevWhy)) {
        why = "previous position is invalid: " + prevWhy;
        return false;
    }

    auto own = [&](char p) { return p != '.' && isWhite(p) == white; };
    auto enemy = [&](char p) { return p != '.' && isWhite(p) != white; };

    auto addMove = [&](int ff, int fr, int tf, int tr, char promote = 0) {
        if (tf < 0 || tf > 7 || tr < 0 || tr > 7 ||
            ff < 0 || ff > 7 || fr < 0 || fr > 7) return;
        char p = prev.at(ff, fr), q = prev.at(tf, tr);
        if (!own(p) || (q != '.' && !enemy(q))) return;

        Board b = prev;
        b.at(tf, tr) = promote ? promote : p;
        b.at(ff, fr) = '.';
        addCandidate(b, stm, partial, known, cand);
    };

    auto slide = [&](int f, int r, int df, int dr) {
        for (int k = 1; k < 8; ++k) {
            int nf = f + df * k, nr = r + dr * k;
            if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
            char q = prev.at(nf, nr);
            if (q == '.') {
                addMove(f, r, nf, nr);
            } else {
                if (enemy(q)) addMove(f, r, nf, nr);
                break;
            }
        }
    };

    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            char p = prev.at(f, r);
            if (!own(p)) continue;

            switch (p) {
                case 'P': case 'p': {
                    int d = white ? 1 : -1;
                    int start = white ? 1 : 6;
                    int promo = white ? 7 : 0;
                    int nr = r + d;

                    // One-square and initial two-square pawn pushes.
                    if (nr >= 0 && nr < 8 && prev.at(f, nr) == '.') {
                        if (nr == promo) addPromotionMoves(prev, stm, partial, known, cand, f, r, f, nr);
                        else addMove(f, r, f, nr);

                        int nr2 = r + 2 * d;
                        if (r == start && nr2 >= 0 && nr2 < 8 && prev.at(f, nr2) == '.')
                            addMove(f, r, f, nr2);
                    }

                    // Normal captures.
                    for (int df : {-1, 1}) {
                        int nf = f + df;
                        if (nf < 0 || nf > 7 || nr < 0 || nr > 7) continue;
                        if (enemy(prev.at(nf, nr))) {
                            if (nr == promo) addPromotionMoves(prev, stm, partial, known, cand, f, r, nf, nr);
                            else addMove(f, r, nf, nr);
                        }
                    }

                    // En-passant recovery.
                    // We do not need the old FEN EP field here: the visual
                    // transition uniquely shows a diagonal pawn move onto an
                    // empty square while removing an adjacent enemy pawn.
                    for (int df : {-1, 1}) {
                        int nf = f + df;
                        if (nf < 0 || nf > 7 || nr < 0 || nr > 7) continue;
                        if (prev.at(nf, nr) != '.') continue;
                        int capturedRank = r;
                        char captured = prev.at(nf, capturedRank);
                        if (captured != (white ? 'p' : 'P')) continue;

                        Board b = prev;
                        b.at(nf, nr) = p;
                        b.at(f, r) = '.';
                        b.at(nf, capturedRank) = '.';
                        addCandidate(b, stm, partial, known, cand);
                    }
                    break;
                }

                case 'N': case 'n': {
                    static const int k[8][2] = {
                        {1,2},{2,1},{-1,2},{-2,1},
                        {1,-2},{2,-1},{-1,-2},{-2,-1}
                    };
                    for (auto &d : k) addMove(f, r, f + d[0], r + d[1]);
                    break;
                }

                case 'B': case 'b':
                    slide(f, r, 1, 1); slide(f, r, 1, -1);
                    slide(f, r, -1, 1); slide(f, r, -1, -1);
                    break;

                case 'R': case 'r':
                    slide(f, r, 1, 0); slide(f, r, -1, 0);
                    slide(f, r, 0, 1); slide(f, r, 0, -1);
                    break;

                case 'Q': case 'q':
                    slide(f, r, 1, 1); slide(f, r, 1, -1);
                    slide(f, r, -1, 1); slide(f, r, -1, -1);
                    slide(f, r, 1, 0); slide(f, r, -1, 0);
                    slide(f, r, 0, 1); slide(f, r, 0, -1);
                    break;

                case 'K': case 'k':
                    for (int df = -1; df <= 1; ++df)
                        for (int dr = -1; dr <= 1; ++dr)
                            if (df || dr) addMove(f, r, f + df, r + dr);
                    break;
            }
        }
    }

    // Castling recovery. Check the starting square and transit square as well
    // as the resulting position; this avoids accepting castling through check.
    if (white && prev.at(4,0) == 'K') {
        if (prev.at(7,0) == 'R' && prev.at(5,0) == '.' && prev.at(6,0) == '.' &&
            !attacked(prev,4,0,false) && !attacked(prev,5,0,false) && !attacked(prev,6,0,false)) {
            Board b = prev;
            b.at(6,0)='K'; b.at(5,0)='R'; b.at(4,0)=b.at(7,0)='.';
            addCandidate(b, stm, partial, known, cand);
        }
        if (prev.at(0,0) == 'R' && prev.at(1,0) == '.' && prev.at(2,0) == '.' && prev.at(3,0) == '.' &&
            !attacked(prev,4,0,false) && !attacked(prev,3,0,false) && !attacked(prev,2,0,false)) {
            Board b = prev;
            b.at(2,0)='K'; b.at(3,0)='R'; b.at(4,0)=b.at(0,0)='.';
            addCandidate(b, stm, partial, known, cand);
        }
    } else if (!white && prev.at(4,7) == 'k') {
        if (prev.at(7,7) == 'r' && prev.at(5,7) == '.' && prev.at(6,7) == '.' &&
            !attacked(prev,4,7,true) && !attacked(prev,5,7,true) && !attacked(prev,6,7,true)) {
            Board b = prev;
            b.at(6,7)='k'; b.at(5,7)='r'; b.at(4,7)=b.at(7,7)='.';
            addCandidate(b, stm, partial, known, cand);
        }
        if (prev.at(0,7) == 'r' && prev.at(1,7) == '.' && prev.at(2,7) == '.' && prev.at(3,7) == '.' &&
            !attacked(prev,4,7,true) && !attacked(prev,3,7,true) && !attacked(prev,2,7,true)) {
            Board b = prev;
            b.at(2,7)='k'; b.at(3,7)='r'; b.at(4,7)=b.at(0,7)='.';
            addCandidate(b, stm, partial, known, cand);
        }
    }

    if (cand.empty()) {
        why = "no legal one-move transition matches recognised squares";
        return false;
    }

    std::vector<Board> uniq;
    for (const Board& b : cand) {
        bool seen = false;
        for (const Board& u : uniq) if (b == u) { seen = true; break; }
        if (!seen) uniq.push_back(b);
    }

    if (uniq.size() != 1) {
        why = "ambiguous recovery: " + std::to_string(uniq.size()) + " legal positions match";
        return false;
    }

    out = uniq[0];
    return true;
}


} // namespace chess
