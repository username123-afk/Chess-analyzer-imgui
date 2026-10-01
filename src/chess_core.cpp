#include "chess_core.h"
#include <cstring>
namespace chess {
Board::Board(){std::fill(sq,sq+64,'.');} bool Board::operator==(const Board&o)const{return std::memcmp(sq,o.sq,64)==0;}
Board startBoard(){Board b; const char*r="RNBQKBNR";const char*p="rnbqkbnr";for(int f=0;f<8;f++){b.at(f,0)=r[f];b.at(f,1)='P';b.at(f,6)='p';b.at(f,7)=p[f];}return b;}
bool isWhite(char p){return p>='A'&&p<='Z';}
bool validate(const Board&b,bool kings,bool,std::string&why){int wk=0,bk=0;for(char p:b.sq){if(std::string("PNBRQKpnbrqk.").find(p)==std::string::npos){why="invalid piece";return false;}if(p=='K')wk++;if(p=='k')bk++;}if(kings&&(wk!=1||bk!=1)){why="missing/duplicate king";return false;}return true;}
std::string fen(const Board&b,char side,const std::string&castling,const std::string&ep){std::string s;for(int r=7;r>=0;r--){int n=0;for(int f=0;f<8;f++){char p=b.at(f,r);if(p=='.')n++;else{if(n){s+=char('0'+n);n=0;}s+=p;}}if(n)s+=char('0'+n);if(r)s+='/';}s+=' ';s+=side;s+=' ';s+=castling;s+=' ';s+=ep;s+=" 0 1";return s;}
}
