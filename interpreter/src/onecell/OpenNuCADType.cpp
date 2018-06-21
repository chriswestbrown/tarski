#include "OpenNuCADType.h"
namespace tarski {
TypeExtensionObj::AMsgManager<OpenNuCADObj> OpenNuCADObj::_manager = TypeExtensionObj::AMsgManager<OpenNuCADObj>();

TypeExtensionObj::LFH<OpenNuCADObj> OpenNuCADObj::A[] = {
  {"num-x-cells",&OpenNuCADObj::numXCells,"returns the number of leaf cells."},
  {"num-t-cells",&OpenNuCADObj::numTCells,"returns the number of cells marked TRUE."},
  {"num-f-cells",&OpenNuCADObj::numFCells,"returns the number of cells marked FALSE."},
  {"print-t-cells",&OpenNuCADObj::printTCells,"prints all leaf cells marked TRUE."},
  {"print-f-cells",&OpenNuCADObj::printFCells,"prints all leaf cells marked FALSE."},
  {"pf-summary",&OpenNuCADObj::pfSummary,"TO APPEAR"},
  {"get-cell",&OpenNuCADObj::getCell,"given label L, returns OCBuilder object for the cell with label L."},
  {"get-var-order",&OpenNuCADObj::getVarOrder,"returns the variable order for this NuCAD."},
  {"plot-all",&OpenNuCADObj::plotAll,"TO APPEAR"},
  {"plot-leaves",&OpenNuCADObj::plotLeaves,"(msg <OpenNuCAD> 'plot-leaves view lab fname) produces an svg-formatted file with name fname that draws the leaf cells of the NuCAD cell with label given by the string lab.  The parameter 'view' is a string describing the portion of the plane to be shown, and the dimensions (in pixels) of the window the plot will appear in.  So, (msg D 'plot-leaves \"-1 3 -2 2 400 400\" \"C\" \"out.svg\") would produce the file out.svg, which would draw cells in the decomposition that fall within -1 < x < 3 and -2 < y < 2.  The plot would be drawn in a 400px-by-400px window."},
  {"show-graph",&OpenNuCADObj::showGraph,"(msg <OpenNuCAD> 'show-graph lab fname) prints a dot-formatted description of the subgraph of the OpenNuCAD structure rooted at the cell with label lab.  What is done with this file is up to the user, but if the file was named foo.dot, for example, the call 'dot -Tsvg foo.dot -o foo.svg' would produce an svg rendering of the graph in the file foo.svg."},
  {"merge-low",&OpenNuCADObj::mergeLow,"TO APPEAR"},
  {"merge-high",&OpenNuCADObj::mergeHigh,"TO APPEAR"},
  {"exprop",&OpenNuCADObj::exprop,"(msg <OpenNuCAD> 'exprop L k) performs 'EasyProjection' onto k-space on the cell with label L. 'EasyProjection' is described in a paper submitted to ISSAC2017.  It tries to project onto lower dimension based purely on the structure of the NuCAD tree."},
  {"tiss",&OpenNuCADObj::tiss,"TO APPEAR"},
  {"def-form",&OpenNuCADObj::defForm,"returns a defining formula for the true cells in the NuCAD."},
  {"refine-cell",&OpenNuCADObj::refineCell,"TO APPEAR"},
  {"negate",&OpenNuCADObj::negate,"negates truth values, so that all TRUE cells are marked FALSE and vice versa."},
  {"locate",&OpenNuCADObj::locate,"given rational point alpha, locate returns the/a cell containing alpha.  If the level of alpha is equal to the level of the NuCAD, there will be a unique cell that contains it.  If alpha has lower dimenion, call it k, then one cell whose intersection with alpha x R^(n-k) is non-empty will be returned, though there may be multiple such cells."},
  {"subtree-level",&OpenNuCADObj::subtreeLevel,"(msg <OpenNuCAD> 'subtree-level L), given label L, returns k that is the maximum split-level of all proper descendents of the cell, or 0 if the cell is leaf."},
  {"projection-resolve",&OpenNuCADObj::projectionResolve,"(msg <OpenNuCAD> 'projection-resolve L1 L2 k), where L1 and L2 are labels of stack sibling cells in a NuCAD whose subtree-levels are <= k, but whose split-level is > k, and modifies the two cells so that one is a FALSE leaf cell, and the other has the property that its subtree's projection onto R^k is the same as the union of the projections of the two original cells' subtrees. "},
  {"project",&OpenNuCADObj::projectStrict,"(msg <OpenNuCAD> 'project k) modifies the OpenNuCAD by projecting down onto R^k, this projection is strict, meaning that if a cell is marked true, there is a solution point over every point in the cell."},
  {"project-loose",&OpenNuCADObj::projectLoose,"(msg <OpenNuCAD> 'project-loose k) modifies the OpenNuCAD by projecting down onto R^k, this projection is not strict, meaning if a cell is marked true, there may be a lower dimensional subset of points in the cell over which there are no solutions."},
  {"union",&OpenNuCADObj::unionOp,"(msg <OpenNuCAD> 'union D) modifies the OpenNuCAD so that it's true cells define the union of the set it originally defines and the set defined by the true cells of OpenNuCAD D."},
  {"test",&OpenNuCADObj::test,"PURELY FOR TESTING EXPERIMENTAL FEATURES"},
  {0,0,"function does not exist"}
 };

void OpenNuCADObj::plotLeaves(const string & wininfo, const string startLabel, std::ostream& out)
{
  istringstream sin(wininfo);
  double xmind, xmaxd, ymind, ymaxd;
  int pixw, pixh;
  if (!(sin >> xmind >> xmaxd >> ymind >> ymaxd >> pixw >> pixh))
    throw TarskiException("OpenNuCAD plot-all requires argument of type str \"xmin xmax ymin ymax pixw pixh\".");
  Word xmin = IEEELBRN(xmind);
  Word ymin = IEEELBRN(ymind);
  double xwd = xmaxd - xmind;
  if (xwd <= 0) throw TarskiException("OpenNuCAD plot-all empty window!");
  double ywd = ymaxd - ymind;
  if (ywd <= 0) throw TarskiException("OpenNuCAD plot-all empty window!");
  Word xw, tmp1; RNFCL2(LBRNRN(IEEELBRN(xwd)),&tmp1,&xw);
  Word yw, tmp2; RNFCL2(LBRNRN(IEEELBRN(ywd)),&tmp2,&yw);
  Word logpixw = ILOG2(pixw);
  Word logpixh = ILOG2(pixh);

  NodeRef start = nucad->getNode(startLabel);
  ONuCADObj::LeafIterator itr = nucad->iterator(start);
  if (true)
  {
    double lineWidth = 0.75 * (1.0/(pixw/xwd) + 1.0/(pixh/ywd))/2.0;
    out << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;	
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" " 
	<< "overflow=\"hidden\" clip=\"auto\" height=\"" << 10 + pixh << "px\" width=\"" << 10 + pixw << "px\" "
	<< "id=\"plot\" onmousedown=\";\" style=\"background-color: #ffffff;\">" << endl;
    out << "<defs>" << endl;
    out << "<style type=\"text/css\" >" << endl
	<< "<![CDATA[" << endl
	<< ".boundaryT { stroke-width: " << lineWidth << "px; fill: rgb(96, 152, 247); fill-opacity: 0.25; stroke: rgb(96, 152, 247); }" << endl
	<< ".boundaryF { stroke-width: " << lineWidth << "px; fill: rgb(96, 152, 247); fill-opacity: 0.00; stroke: rgb(96, 152, 247); }" << endl
	<< "]]>" << endl
	<< "</style>" << endl;

    out << "</defs>" << endl;

    out << "<g transform=\"scale(1,-1)\">" << endl;
    out << "<g transform=\"translate(0," << -(10 + pixh) << ")\">" << endl;
    out << "<g transform=\"translate(5,5)\">" << endl;
    //    out << "<g transform=\"translate(" << pixw/2.0 << "," << pixh/2.0 << ")\">" << endl;
    out << "<g transform=\"scale(" << pixw/xwd << "," << pixh/ywd << ")\">" << endl;
    out << "<g transform=\"translate(" << -xmind << "," << -ymind << ")\">" << endl;
 

    while(itr.hasNext())
    {
      NodeRef n = itr.next();
      out << "<polyline id=\"" << n->getLabel() << "\""
	  << " class=\"" << (n->getTruthValue() == TRUE ? "boundaryT" : "boundaryF") << "\"" 
	  << " points=\"";
      n->getData()->getCell()->approx2D(xmin,ymin,xw,yw,logpixw,logpixh,out);      
      out << "\">" << endl
	  << "</polyline>" << endl;
    }
    out << "</g>" << endl;
    out << "</g>" << endl;
    out << "</g>" << endl;
    out << "</g>" << endl;
    out << "</g>" << endl;
    out << "</svg>" << endl;
  }
}

SRef OpenNuCADObj::project(vector<SRef>& args, bool strictFlag)
{
    NumRef k;
    if (args.size() < 1 || ((k = args[0]->num()), k.is_null())) 
      return new ErrObj("OpenNuCAD project requires argument of type num.");
    if (k->denominator() != 1)
      return new ErrObj("OpenNuCAD project requires an integer argument.");
    int K = RNROUND(k->getVal());

    NodeRef res;

    do {
      nucad->expropDownTo(nucad->getRoot(),K,strictFlag);
      res = nucad->getRoot()->lowerMostNodeWithSubtreeLevelExceedingK(K);
      if (res.is_null()) break;
      NodeRef n1 = res->XYchild, n2;
      int m = res->XYchild->getSplitLevel();
      if (res->Lchild.size() > 0 && res->Lchild.back()->getSplitLevel() == m )
	n2 = res->Lchild.back();
      if (n2.is_null() || (res->Uchild.size() > 0  && res->Uchild.back()->getSplitLevel() == m && !n2->hasChildren()))
	n2 = res->Uchild.back();      
      nucad->projectionResolve(n1,n2,K,nucad->getDim());
      n2->setTruthValue(FALSE);
      n2->XYchild = NULL;
      n2->Lchild.clear();
      n2->Uchild.clear();
    }while(!res.is_null());
    return new SObj();
  }

}//end namespace tarski
