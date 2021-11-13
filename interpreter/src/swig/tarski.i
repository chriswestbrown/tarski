%module tarski
%include <std_string.i>
%{
extern void TARSKIINIT(int numcells, int timeout);
extern std::string TARSKIEVAL(std::string input);
extern void TARSKIEND();
%}
extern void TARSKIINIT(int numcells, int timeout);
extern std::string TARSKIEVAL(std::string input);
extern void TARSKIEND();
