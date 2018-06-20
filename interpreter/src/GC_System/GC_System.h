/***********************************************************
 *** This file essentially defines a garbage collection
 *** system for C++ classes.  Specifically, the class
 *** GC_Obj is defined, and the class GC_Hand is defined.
 *** If you want to define a class MyClass which is to be
 *** garbage collected, you do so as follows:
 ***
 *** 1) Define class MyClass as derived from GC_Obj.
 *** 2) Access objects of type MyClass through handles
 ***    of type GC_Hand<MyClass>, or from some class derived
 ***    from GC_Hand<MyClass>.
 *** 
 *** This system does, of course, imply that all garbage
 *** collected objects are allocated on the heap.  C'est la
 *** vie.  Note that the class T appearing in GC_Hand<T>
 *** MUST BE DERIVED FROM CLASS GC_Obj!!!  Otherwise the
 **  whole thing breaks!
 ***********************************************************/
//#include "GC_System_Header.h"
#include "GC_System_Header.icc"
