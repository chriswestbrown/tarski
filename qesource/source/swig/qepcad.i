%module qepcad
%include <std_string.i>
%{
extern void mainLIB(int numcells, int timeout);
extern std::string PCLIB(std::string input);
extern void PRINTBANNER();
%}
extern void mainLIB(int numcells, int timeout);
extern std::string PCLIB(std::string input);
extern void PRINTBANNER();
