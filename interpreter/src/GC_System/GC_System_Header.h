#ifndef _GC_System_
#define _GC_System_
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
 **
 **  This version requires the calling code to define and
 **  initialize: uint64 GC_Obj::next_tag, GC_Obj::num_deleted.
 **  The purpose is that each garbage collected object has
 **  a unique "tag" that is even more unique (if I may say
 **  that) than a pointer, because two objects can have the
 **  same pointer value if they simply exist at different
 **  times during program execution.
 ***********************************************************/
#include <iostream>
#include <cstdlib>
using namespace std;

#ifndef NULL
#define NULL 0
#endif

typedef unsigned long long uint64;

/***********************************************************/
/*** class definition for GC_Obj ***************************/
/***********************************************************/
class GC_Obj
{
public:
  int ref_count;
  static uint64 next_tag, num_deleted;
  uint64 tag;
  GC_Obj() : ref_count(0), tag(next_tag++) { }
  virtual ~GC_Obj() { ++num_deleted; }
};

/***********************************************************/
/*** definition for templated class GC_Hand. ***************/
/***********************************************************/
template<class GC_Obj_Ext>
class GC_Hand
{
  GC_Obj_Ext* p;
public:
  //--- Constructors --------------------------------------//
  inline GC_Hand();
  inline GC_Hand(const GC_Hand &H);
  inline GC_Hand(GC_Obj_Ext *ptr);
  template<class T>
  inline GC_Hand(const GC_Hand<T> &H);

  //--- Destructors ---------------------------------------//
  inline ~GC_Hand() { free(); }
  inline void free();
  //--- Assignments ---------------------------------------//
  inline GC_Hand& operator=(const GC_Obj_Ext* &ptr);
  inline GC_Hand& operator=(const GC_Hand& H);
  //--- Dereferenceing ------------------------------------//
  inline GC_Obj_Ext& operator*() const;
  inline GC_Obj_Ext* operator->() const;
  //--- Information ---------------------------------------//
  inline int ref_value() const;
  inline int is_null() const;
  void* vpval() const; // returns address of ref'd object
  template<class A>
  inline bool identical(const GC_Hand<A> &a) const;
};

#endif
