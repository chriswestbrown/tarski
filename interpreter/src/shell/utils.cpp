#include "utils.h"
#include <sstream>
#include <cstdlib>
#include <vector>
#ifndef __MINGW32__
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include <unistd.h>
using namespace std;

namespace tarski {
  
int getTermWidth()
{
#ifndef __MINGW32__
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int cols = w.ws_col;
  int rows = w.ws_row;
#else
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#endif
  if (cols == 0) {
    cols = 80; // Set a default value if no meaningful data was obtained.
    // This should never happen.
    // But Emscripten-3.1.22 seems to optimize out the value of cols if it is
    // not edited. So we do some dummy operation here to work around the issue.
    }
  return cols;
}

void writeToFit(const string& in, ostream& out, int N, int lineWrapIndent)
{
  istringstream sin(in);
  writeToFit(sin,out,N,lineWrapIndent);
}

void write(ostream& out, vector<char>& v)
{
  for(int i = 0; i < v.size(); i++)
    out << v[i];
  v.clear();
}

void rep(ostream& out, char c, int n) { for(int i = 0; i < n; i++) out << c; }

void writeToFit(istream& in, ostream& out, int N, int lineWrapIndent)
{
  int currLineLength = 0;
  char c = '\n', prevc = '\n';
  vector<char> waiting;
  while((c == ' ' || (prevc = c)) && (c = in.get()) && in)
  {
    //out << "c = " << int(c) << " and prevc = " << int(prevc) << endl;
    if (c == '\n')
    {
      if (waiting.size() + currLineLength <= N)
      {
	write(out,waiting);
	out << '\n';
	currLineLength = 0;
      }
      else
      {
	out << '\n';
	rep(out,' ',lineWrapIndent); currLineLength = 6;
	write(out,waiting);
	out << '\n';
	currLineLength = 0;
      }
    }
    
    else if (c == ' ')
    {
      if (prevc == '\n')
      {
	out << c;
	currLineLength++;
      }
      else if (waiting.size() == 0)
      {
	if (0 < currLineLength && currLineLength < N) { out << c; currLineLength++; }
	else if (currLineLength == N) { out << '\n'; currLineLength = 0; }
      }
      else if (waiting.size() + currLineLength < N)
      {
	int t = waiting.size();
	write(out,waiting);	
	out << c;
	currLineLength += t + 1;
      }
      else
      {
	out << '\n';
	currLineLength = 0;
	rep(out,' ',lineWrapIndent);  currLineLength = 6;
	int t = waiting.size();
	write(out,waiting);
	if (t + currLineLength < N) { out << c; currLineLength += t + 1; c = 'x'; }
	else { out << '\n'; currLineLength = 0; c = '\n';}
      }
    }
    else
    {
      waiting.push_back(c);
    }
  }
  if (waiting.size() > 0) { write(out,waiting); }
}

}//end namespace tarski
