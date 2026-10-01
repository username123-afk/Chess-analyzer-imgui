#pragma once
#include <string>
#include <vector>
namespace chess {
struct Board { char sq[64]; Board(); char& at(int f,int r){return sq[r*8+f];} char at(int f,int r) const{return sq[r*8+f];} bool operator==(const Board&o)const; };
Board startBoard(); bool isWhite(char p); bool validate(const Board& b,bool requireKings,bool requirePawns,std::string& why); std::string fen(const Board& b,char side,const std::string& castling="-",const std::string& ep="-");
}
