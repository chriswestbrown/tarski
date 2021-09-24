%module tarski
%include <std_string.i>
%{
extern void mainLIB(int numcells, int timeout);
extern std::string PCLIB(std::string input);
extern void ENDTARSKILIB();
%}
extern void mainLIB(int numcells, int timeout);
extern std::string PCLIB(std::string input);
extern void ENDTARSKILIB();
