#pragma once
#include <string>
#include <vector>
namespace chess {
struct Board { char sq[64]; Board(); char& at(int f,int r){return sq[r*8+f];} char at(int f,int r) const{return sq[r*8+f];} bool operator==(const Board&o)const; };
Board startBoard(); bool isWhite(char p);
bool validate(const Board& b,bool requireKings,bool requirePawns,std::string& why);
bool validateFull(const Board& b,char stm,std::string& why);
std::string sanitizeCastling(const Board& b,const std::string& c);
std::string sanitizeEp(const Board& b,char stm,const std::string& ep);
std::string fen(const Board& b,char side,const std::string& castling="-",const std::string& ep="-");

// Recover a position after a missed/blurred frame. `partial` is a board in chess
// coordinates, `known[i]` says that square was actually recognised. The function
// considers ordinary legal one-move transitions from prev and returns the unique
// candidate matching every recognised square.
bool recoverOneMove(const Board& prev, char stm, const Board& partial, const bool known[64], Board& out, std::string& why);
}
