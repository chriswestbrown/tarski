/* Write a postscript plot of the CAD. */
#include "rend.h"
#include "Mapper.h"

void SVG_HEADER(double lineWidth, double x1, double y1, double x2, double y2, double offset, ostream& out, double D)
{
  string c1 = "rgb(96, 152, 247)";
  string c2 = "rgb(245, 191, 66)";
  string c3 = "rgb(255, 0, 0)";
  int BBx1 = int(floor(x1 + D));
  int BBy1 = int(floor(y1 + D));
  int BBx2 = int(ceil(x2 + 2*offset + D));
  int BBy2 = int(ceil(y2 + 2*offset + D));
  out << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;	
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" " 
      << "overflow=\"hidden\" clip=\"auto\" height=\"" << BBy2 - BBy1 << "px\" width=\"" << BBx2 - BBx1 << "px\" "
      << "id=\"plot\" onmousedown=\";\" style=\"background-color: #ffffff;\">" << endl;
  out << "<defs>" << endl;
  out << "<style type=\"text/css\" >" << endl
      << "<![CDATA[" << endl
      << ".c10T { stroke-width: " << lineWidth << "px; fill: " << c1 << "; fill-opacity: 0.0 ; stroke: " << c1 << "; }" << endl
      << ".c10F { stroke-width: " << lineWidth << "px; fill: " << c2 << "; fill-opacity: 0.0; stroke: " << c2 << "; }" << endl
      << ".c10U { stroke-width: " << lineWidth << "px; fill: " << c3 << "; fill-opacity: 0.0 stroke: " << c3 << "; }" << endl
      << ".c11T { stroke-width: " << lineWidth << "px; fill: " << c1 << "; fill-opacity: 0.25; stroke: " << c1 << "; stroke-opacity: 0; }" << endl
      << ".c11F { stroke-width: " << lineWidth << "px; fill: " << c2 << "; fill-opacity: 0.00; stroke: " << c2 << "; stroke-opacity: 0; }" << endl
      << ".c11U { stroke-width: " << lineWidth << "px; fill: " << c3 << "; fill-opacity: 0.25; stroke: " << c3 << "; stroke-opacity: 0; }" << endl
      << ".c00T { stroke-width: " << lineWidth << "px; fill: " << c1 << "; fill-opacity: 1.0 ; stroke: " << c1 << "; }" << endl
      << ".c00F { stroke-width: " << lineWidth << "px; fill: " << c2 << "; fill-opacity: 1.0; stroke: " << c2 << "; }" << endl
      << ".c00U { stroke-width: " << lineWidth << "px; fill: " << c3 << "; fill-opacity: 1.0 stroke: " << c3 << "; }" << endl
      << ".c01T { stroke-width: " << lineWidth << "px; fill: " << c1 << "; fill-opacity: 0.25; stroke: " << c1 << "; stroke-opacity: 1; }" << endl
      << ".c01F { stroke-width: " << lineWidth << "px; fill: " << c2 << "; fill-opacity: 0.00; stroke: " << c2 << "; stroke-opacity: 1; }" << endl
      << ".c01U { stroke-width: " << lineWidth << "px; fill: " << c3 << "; fill-opacity: 0.25; stroke: " << c3 << "; stroke-opacity: 1; }" << endl
      << "]]>" << endl
      << "</style>" << endl; 
  out << "</defs>" << endl;
  out << "<g transform=\"translate(0," << (BBy2 - BBy1) << ")\">" << endl;
  out << "<g transform=\"scale(1,-1)\">" << endl;
  out << "<g transform=\"translate(" << offset << "," << offset << ")\">" << endl;
}


void WRITE_SVG(Rend_Cell &M, Rend_Win &W, Word L, Word e, Word P, ostream &out, bool c, bool z)
{
  Word l,r;
  
  /****************************************
  *** Initialize     
  *****************************************/
  l = LAST(L);
  r = FIRST(L);  
  Rend_Win Wp(W);

  /****************************************
  *** Write ps header
  *****************************************/
  double D = 20;
  double offset = 0;
  double x1 = 0, y1 = 0,
    x2 = W.pixdim.x,
    y2 = W.pixdim.y,
    offsetp = offset * 1000 / double(W.pixdim.x);
  double lineWidth = 1000/double(W.pixdim.x);
  out.setf(ios::showpoint);
  out << setprecision(4);
  SVG_HEADER(lineWidth,x1,y1,x2,y2,offset,out,D);
  out << endl
      << "<!-- Here I'm scaling so that a 1000 x 1000 box fits the Bounding Box. -->" << endl
      << "<g transform=\"scale(" << x2/1000 << "," << y2/1000 << ")\">" << endl;

  // Write cells
  Mapper Q(0,0,1000,1000,Wp);
  Word i,j;  
  // 1D sectors
  for(i = l; i <= r; i += 2)
    if (M[i].child.size() == 2) { // no real children in stack!
      M[i].out_descrip_svg_standard(Wp,out,Q);
    }
  // 1D sections
  for(i = l+1; i <= r; i += 2)
    if (M[i].child.size() == 2) { // no real children in stack!
      M[i].out_descrip_svg_standard(Wp,out,Q);
    }  
  // Sectors over sectors
  for(i = l; i <= r; i += 2)
    for(j = 1; j < M[i].child.size()-1; j += 2)
      M[i][j].out_descrip_svg_standard(Wp,out,Q);
  // Sectors over sections
  for(i = l+1; i < r; i += 2)
    for(j = 1; j < M[i].child.size()-1; j += 2)
      M[i][j].out_descrip_svg_standard(Wp,out,Q);
  // Sections over sectors
  for(i = l; i <= r; i += 2)
    for(j = 2; j < M[i].child.size()-1; j += 2)
      M[i][j].out_descrip_svg_standard(Wp,out,Q);
  // Sections over sections
  for(i = l+1; i < r; i += 2)
    for(j = 2; j < M[i].child.size()-1; j += 2)
      M[i][j].out_descrip_svg_standard(Wp,out,Q);
  


  /****************************************
   *** Footer
   *****************************************/
  out << "</g>" << endl;
  out << "</g>" << endl;
  out << "</g>" << endl;
  out << "</g>" << endl;
  out << "</svg>" << endl;
}
  
