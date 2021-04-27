%module qepcad
%include <std_string.i>
%{
extern void mainLIB(int numcells, int timeout);
extern std::string PCLIB(std::string input);
extern void PRINTBANNER();
extern std::string SLFQLIB(std::string str_formula, std::string str_assumptions);
extern void ENDQEPCADLIB();
%}
extern void mainLIB(int numcells, int timeout);
extern std::string PCLIB(std::string input);
extern void PRINTBANNER();
extern std::string SLFQLIB(std::string str_formula, std::string str_assumptions);
extern void ENDQEPCADLIB();
