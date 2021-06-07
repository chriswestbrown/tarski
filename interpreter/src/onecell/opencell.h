#ifndef _OC_OPEN_CELL_
#define _OC_OPEN_CELL_

#include "varorder.h"
#include "cellbound.h"

namespace tarski {
/*
OpenCell Data Structure
- should be able to give you the CellBound for any level i
- should be able to give you the point alpha that is its sample point
- given rational point PT, should be able to tell you whether or not the point is in this cell
- should be able to return a defining formula for the cell
 */


class OpenCellObj; typedef GC_Hand<OpenCellObj> OpenCellRef;
class OpenCellObj : public GC_Obj
{
 private:
  VarOrderRef X; // gets PolyManager from this
  GCWord alpha;
  std::vector<CellBoundRef> CB;

 public:
  OpenCellObj(VarOrderRef _x, Word _alpha, int dim);
  int dimension() { return CB.size() - 1; }
  void setBound(int i, CellBoundRef b) { CB[i] = b; }
  CellBoundRef& get(int i) { return CB[i]; } // returns the bounds for that level
  void mergeBound(int i, CellBoundRef B) { CB[i] = merge(get(i),B); } //B must be an i-level bound
  PolyManager* getPolyManager() { return X->getPolyManager(); }
  VarOrderRef getVarOrder() { return X; }
  Word getAlpha() { return alpha; }

  // Given rational point PT and integer level, dimension of PT >= level
  // returns true iff projection of PT onto dimension level is contained
  // withing the induced cell of that level.
  bool rationalPointInCellQuery(Word PT, int level);

  // Given rational point PT, returns true iff m = level PT and n = level cell and
  // a) m >= n and the projection of PT onto n-space is contained within the cell, or
  // b) m <  n and PT is contained within the induced cell of level n.
  bool rationalPointInCellQuery(Word PT);

  // Given rational point PT of level m < level of the cell (call it n)
  // such that PT satisfies rationalPointInCellQuery(PT),
  // return a rational point of level n whose first m coordinates are PT,
  // and which lies within the cell
  Word choosePointCompletion(Word PT);

  // Given rational point PT of level m < level of the cell (call it n)
  // such that PT satisfies rationalPointInCellQuery(PT),
  // choose a rational point of level n whose first m coordinates are PT,
  // and which lies within the cell, and change this cell's sample point
  // to this new point.
  // The new point is returned, if a change is made, else NIL.
  Word moveSamplePoint(Word PT);

  class Point2D 
  { 
  public: 
    double x, y; 
    GCWord exactx;
    double linearFactor;
    int level;
    int inViewWindow; // -1 means below window, 0 means in window, 1 means above window
    Point2D(double x, double y, Word exactx, int inViewWindow) 
    {       
      this->x = x; this->y = y; this->exactx = exactx; this->inViewWindow = inViewWindow;
      level = 0;
    } 
  };

  Point2D makePoint2D(Word xm, int xdir, IntPolyRef p, int index, int ydir,
		      int yRefine,
		      Word ymin, double ymind, Word ymax, double ymaxd)
  {
    double xmd = approximateLBRN(xm,xdir), ymd;
    IntPolyRef q = getVarOrder()->partialEval(p,LIST1(LBRNRN(xm)),1);
    RealRootIUPRef ym = RealRootIsolateSquarefree(q)[index-1];


    /* TODO: need to check the y coordinate with respect to the window in the cartesian plane. */
    int inViewWindow;
    if (ym->compareToLBRN(ymax) >= 0) { ymd = ymaxd; inViewWindow = 1; }
    else if (ym->compareToLBRN(ymin) <= 0) { ymd = ymind; inViewWindow = -1; }
    else { ym->refineTo(yRefine); ymd = ym->approximate(ydir); inViewWindow = 0; }
    return Point2D(xmd,ymd,xm,inViewWindow);
  }

  Point2D split(Point2D &A, Point2D &B, int xdir, IntPolyRef p, int index, int ydir,
		      int yRefine,
		      Word ymin, double ymind, Word ymax, double ymaxd)
  { 
    Word xm = LBRNP2PROD(LBRNSUM(A.exactx,B.exactx),-1);
    Point2D M = makePoint2D(xm,0,p,index,ydir,yRefine,ymin,ymind,ymax,ymaxd);
    M.level = 1 + std::max(A.level,B.level);
    M.linearFactor = (B.y - 2*M.y + A.y)/(B.x - A.x);
    return M;
  }

  void pinPoints(Point2D A, Point2D B,
		 IntPolyRef p, int index,
		 std::vector<Point2D> &V,
		 int dir, // roundint dir for the y-coordinates
		 int yRefine,
		 Word ymin, double ymind, Word ymax, double ymaxd,
		 double originalXWidth
		 )
  {
    if (std::max(A.level,B.level) > 10) return;
    bool minWidthFlag = B.x - A.x > originalXWidth/8;
    Point2D M = split(A,B,0,p,index,dir,yRefine,ymin,ymind,ymax,ymaxd);
    if (fabs(M.linearFactor) < 0.001*exp(M.level) && !minWidthFlag) return;
    pinPoints(A,M,p,index,V,dir,yRefine,ymin,ymind,ymax,ymaxd,originalXWidth);
    V.push_back(M);
    pinPoints(M,B,p,index,V,dir,yRefine,ymin,ymind,ymax,ymaxd,originalXWidth);
  }


  // The idea here is that we ultimately want to plot to a pixel grid of approximately
  // 2^logpixw x 2^logpixh, and we'd like to show a window in the cartesian plane 
  // with lower-left corner (xmin,ymin) and with width 2^logw, and height 2^logh.
  void approx2D(Word xmin, Word ymin, int logw, int logh, int logpixw, int logpixh,
		std::ostream& out
		)
  {
    int xRefine = logw - logpixw - 4;
    int yRefine = logh - logpixh - 4;

    Word xmax = LBRNSUM(xmin,LBRN(1,-logw));
    Word ymax = LBRNSUM(ymin,LBRN(1,-logh));
    double xmind = approximateLBRN(xmin,-1);
    double xmaxd = approximateLBRN(xmax,+1);
    double ymind = approximateLBRN(ymin,-1);
    double ymaxd = approximateLBRN(ymax,+1);

    // get left and right endpoints for 1D base
    CellBoundRef B1 = get(1);
    CellBoundRef B2 = get(2);
    RealAlgNumRef a = B1->getBoundNumber(-1);
    RealAlgNumRef b = B1->getBoundNumber(+1);

    // figure out if this cell is left/right of view window; return if out of view
    int xpos = 0;
    if (b->compareToLBRN(xmin) <= 0) { xpos = -1; }
    else if (a->compareToLBRN(xmax) >= 0) { xpos = +1; }
    if (xpos != 0) return;

    // std::set xlp and xup to the left-most and right-most LBRNs in our intervals of x-coordinates
    b->refineTo(xRefine);
    a->refineTo(xRefine);
    Word xlp, xup;
    if (a->compareToLBRN(xmin) < 0)
      xlp = xmin;
    else
    {
      Word xl = a->LBRNPointAbove();
      xlp = EQUAL(a->LBRNPointAbove(),a->LBRNPointBelow()) ? LBRNSUM(xl,LBRNP2PROD(LBRN(1,1),xRefine)) : xl;
    }
    if (b->compareToLBRN(xmax) > 0)
      xup = xmax;
    else
    {
      Word xu = b->LBRNPointBelow();
      xup = EQUAL(b->LBRNPointAbove(),b->LBRNPointBelow()) ? LBRNDIF(xu,LBRNP2PROD(LBRN(1,1),xRefine)) : xu;
    }

    // get upper bound points
    std::vector<Point2D> V;
    {
      int index = B2->getBoundIndex(1);
      if (index == 0) // upper bound is +infty
      {
	V.push_back(Point2D(approximateLBRN(xlp,+1),ymaxd,xlp,1));
	V.push_back(Point2D(approximateLBRN(xup,-1),ymaxd,xup,1));
      }
      else
      {
	IntPolyRef u = B2->getBoundDefPoly(1);
	Point2D A = makePoint2D(xlp,1,u,index,-1,yRefine,ymin,ymind,ymax,ymaxd);
	Point2D B = makePoint2D(xup,-1,u,index,-1,yRefine,ymin,ymind,ymax,ymaxd);
	V.push_back(A);
	pinPoints(A,B,u,index,V,-1,yRefine,ymin,ymind,ymax,ymaxd,xmaxd - xmind);
	V.push_back(B);
      }
    }

    // get lower bound points
    std::vector<Point2D> W;
    {
      int index = B2->getBoundIndex(-1);
      if (index == 0) // lower bound is -infty
      {
	W.push_back(Point2D(approximateLBRN(xlp,+1),ymind,xlp,1));
	W.push_back(Point2D(approximateLBRN(xup,-1),ymind,xup,1));
      }
      else
      {
	IntPolyRef l = B2->getBoundDefPoly(-1);
	Point2D A = makePoint2D(xlp,1,l,index,+1,yRefine,ymin,ymind,ymax,ymaxd);
	Point2D B = makePoint2D(xup,-1,l,index,+1,yRefine,ymin,ymind,ymax,ymaxd);
	W.push_back(A);
	pinPoints(A,B,l,index,W,+1,yRefine,ymin,ymind,ymax,ymaxd,xmaxd - xmind);
	W.push_back(B);
      }
    }

    for(int i = 0; i < V.size(); i++)
      out << V[i].x << '\t' << V[i].y << std::endl;
    for(int i = W.size()-1; i >= 0; i--)
      out << W[i].x << '\t' << W[i].y << std::endl;
    out << V[0].x << '\t' << V[0].y << std::endl;
    out << std::endl;
  }



  // returns a defining formula for level n
  // if no level is specified, returns a defining
  // formula for the level of the OpenCell
  std::string definingFormula(int n = -1)
  {
    if (n < 0) { n = dimension(); }
    std::ostringstream sout;
    sout << "[ ";
    bool andBefore = false;
    for(int i = n; i > 0; --i)
    {
      std::string s = CB[i]->toStrFormula(getPolyManager(),(*X)[i]);
      if (s != "") { sout << (andBefore ? " /\\ " : "") << s << ' ';  }
      andBefore = andBefore || s != "";
    }
    sout << "]";
    return sout.str();
  }

  
  void debugPrint(std::ostream &out, int n = -1);

  void debugPrint() { debugPrint(std::cout,-1); }
  void debugPrint2();


  void writeAlpha()
  {
    SWRITE("(");
    for(Word t = getAlpha(); t != NIL; t = RED(t))
    {
      RNWRITE(FIRST(t));
      if (RED(t) != NIL) SWRITE(" ");
    }
    SWRITE(")");
  }
};
}//end namespace tarski
#endif
