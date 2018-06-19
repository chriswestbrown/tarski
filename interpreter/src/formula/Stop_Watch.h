/**********************************************************
****
****  Interface and implementation for class Stop_Watch.
****
****  Use an object of type Stop_Watch to time program
****  execution.  Method Start() starts the timer, method
****  Stop() stops it. Multiple Start()/Stop() sequences
****  are allowed -- a running total is kept of the elapsed
****  times between Start()'s and Stop()'s.  Method Reset()
****  sets the running total to zero.  Method Time() returns
****  the running total as a double.  The unit of measure is
****  seconds.  Note that there is a limit on the accuracy
****  of time measurements which is inherent in clock(),
****  the system call Stop_Watch uses to measure times.  Time
****  measured is the amount of time spent executing in user
****  mode.
****                 Chris Brown 1/2000
****
****  P.S. If you try a Stop() without a corresponding
****       Start() ... you're on your own! ;-)
****
***********************************************************/
#ifndef _Stop_Watch_
#define _Stop_Watch_
#include <time.h>


/**********************************************************
** Definition of class Stop_Watch
***********************************************************/
class Stop_Watch
{
private:
  /***********************
  ** Data Members
  ************************/
  int totals, memory;

public:
  /***********************
  ** Member Functions
  ************************/
  //---- Constructor
  Stop_Watch() { Reset(); }

  //---- Reset()
  void Reset()
  {
    memory = 0;
    totals = 0;
  }

  //---- Start()
  void Start()
  {
    memory = clock();
  }

  //---- Stop()
  void Stop()
  {
    totals += clock() - memory;
  }

  //---- Time()
  double Time()
  {
    return (double)totals / CLOCKS_PER_SEC;
  }
};

#endif /* _Stop_Watch_ */
