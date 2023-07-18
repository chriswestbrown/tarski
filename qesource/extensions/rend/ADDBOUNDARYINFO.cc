#include "rend.h"

int btype(int tl, int t, int tr) {
  //cerr << "[" << tl <<"," << t << "," << tr << "]" << endl;
  return tl != tr ? 1 : (tl != t ? 2 : 0); 
}

// 0 means not a boundary, 1 means t1 boundary, 2 means t2 boundary
// NOTE: remember that M[i][0] is the "-infty section" and M[i][M[i].child.size()-1] is the "+infty section"
int boundaryType(Rend_Cell& M, int i, int j)
{
  if (i%2==1 && j%2==1) return 0;
  else if (i%2==1 && j%2==0) {
    // cerr << "(" << i << "," << j << ") : " << M[i][j-1].truth << " " << M[i][j].truth << " " << M[i][j+1].truth << endl;
    return btype(M[i][j-1].truth,M[i][j].truth,M[i][j+1].truth);
  }
  else if (i%2==0 && j%2==1) {
    if (j == 1) { // First sector
      // Set jl so (i-1,jl) is the index of left neighbor
      int jl = 1;
      while(jl < M[i-1].child.size() && M[i-1][jl + 1].neighbor_r()->array_index() == 0)
	jl+=2;

      // Set jr so (i+1,jr) is the index of right neighbor
      int jr = 1;
      while(jr < M[i+1].child.size() && M[i+1][jr + 1].neighbor_l()->array_index() == 0)
	jr+=2;

      return btype(M[i-1][jl].truth,M[i][j].truth,M[i+1][jr].truth);
    }
    else if (j == M[i].child.size() - 2) { // Last sector
      int jl = M[i-1].child.size() - 2;
      while(jl > 1 && M[i-1][jl-1].neighbor_r()->array_index() == M[i].child.size()  - 1)
	jl-=2;
      
      int jr = M[i+1].child.size() - 2;
      while(jr > 1 && M[i+1][jr-1].neighbor_l()->array_index() == M[i].child.size()  - 1)
	jr-=2;

      // cerr << "B: " << i << "," << j << " ";
      return btype(M[i-1][jl].truth,M[i][j].truth,M[i+1][jr].truth);
    }
    else {
      // cerr << "C: " << i << "," << j << " ";
      int jlLow = 0, jlHigh = M[i-1].child.size()- 1;
      for(int k = 2; k < M[i-1].child.size()- 1; k+=2) {
	int jp = M[i-1].child[k].neighbor_r()->array_index();
	if (jp < j) { jlLow = k; }
      }
      if (jlLow + 2 < jlHigh) { jlHigh = jlLow + 2; }

      int jrLow = 0, jrHigh = M[i+1].child.size()- 1;
      for(int k = 2; k < M[i+1].child.size()- 1; k+=2) {
	int jp = M[i+1].child[k].neighbor_l()->array_index();
	if (jp < j) { jrLow = k; }
      }
      if (jrLow + 2 < jrHigh) { jrHigh = jrLow + 2; }
      //      cerr << jlLow << " " << jlHigh << " : " << jrLow << " " << jrHigh << endl; 
      return btype(M[i-1][jlLow+1].truth,M[i][j].truth,M[i+1][jrLow+1].truth);
    }
  }
  else { // single point cells
    int cl[3] = {0,0,0}; // count false, true, other
    int tlow = M[i][j-1].truth;
    int t = M[i][j].truth;
    int thigh = M[i][j+1].truth;
    cl[tlow == FALSE ? 0 : (tlow == TRUE ? 1 : 2)]++;
    cl[thigh == FALSE ? 0 : (thigh == TRUE ? 1 : 2)]++;
    // check left neighbors
    int jlhmax = M[i-1].child.size()- 1;
    int jlLow = 0, jlHigh = jlhmax;
    for(int k = 2; k < M[i-1].child.size()- 1; k+=2) {
      int jp = M[i-1].child[k].neighbor_r()->array_index();
      if (jp < j) { jlLow = k; }
      if (j < jp && jlHigh == jlhmax) { jlHigh = k; }
    }
    if (jlLow + 1 == jlHigh) { jlLow++; jlHigh--; }
    else { jlLow+=2; jlHigh-=2; }
    for(int k = jlLow; k <= jlHigh; k++) {
      int t = M[i-1][k].truth;
      cl[t == FALSE ? 0 : (t == TRUE ? 1 : 2)]++;
    }

    // check right neighbors
    int jrhmax = M[i+1].child.size()- 1;
      int jrLow = 0, jrHigh = jrhmax;
    for(int k = 2; k < M[i+1].child.size()- 1; k+=2) {
      int jp = M[i+1].child[k].neighbor_l()->array_index();
      if (jp < j) { jrLow = k; }
      if (j < jp && jrHigh == jrhmax) { jrHigh = k; }
    }
    if (jrLow + 1 == jrHigh) { jrLow++; jrHigh--; }
    else { jrLow+=2; jrHigh-=2; }
    for(int k = jrLow; k <= jrHigh; k++) {
      int t = M[i+1][k].truth;
      cl[t == FALSE ? 0 : (t == TRUE ? 1 : 2)]++;
    }


    // This little bit says it's type 2 if truth above and below disagree
    {
      int bt = btype(M[i][j-1].truth,M[i][j].truth,M[i][j+1].truth);
      if (bt == 2) {
	// cerr << "A: (" << i << "," << j << ") " << bt << endl;
	return 2;
      }
    }
    // special case to make curves look like curves!
    // if (jlLow == jlHigh && jrLow == jrHigh && jlLow % 2 == 0 && jrLow % 2 == 0 &&
    // 	M[i-1][jlLow].btype == M[i+1][jrLow].btype &&
    // 	M[i-1][jlLow].truth == M[i+1][jrLow].truth && M[i-1][jlLow].truth == M[i][j].truth)
    //   return M[i-1][jlLow].btype;
    
    if (cl[0] > 0 && cl[1] > 0 || cl[0] > 0 && cl[2] > 0 || cl[1] > 0 && cl[2] > 0) {
      // cerr << "B: (" << i << "," << j << ") " << 1
      // 	   << "[" << cl[0] << "," << cl[1] << "," << cl[2] << "]"
      // 	   << endl;      
      return 1;
    }
    else {
      return (cl[t == FALSE ? 0 : (t == TRUE ? 1 : 2)] > 0) ? 0 : 2;
    }
    
  }
}


// (i,j) is a single-point cell of positive boundary type
// return true if we don't want a dot for it because the
// boundary-ness is implied by its neighbors
// NOTE: must be sure the boundary types for everything already filled in!
bool decideNoDot(Rend_Cell& M, int i, int j) {
  // cerr << "(" << i << "," << j << ") : ";
  
  // check left neighbors
  int bcl[3] = {0,0,0}; // count bt0, bt1, bt2
  int jl = -1;
  for(int k = 2; k < M[i-1].child.size()- 1; k+=2) {
    if (j == M[i-1].child[k].neighbor_r()->array_index()) {
      int bt = M[i-1].child[k].btype;
      bcl[bt]++;
      if (bt > 0) { jl = k; }
    }
  }
    
  // check right neighbors
  int bcr[3] = {0,0,0}; // count bt0, bt1, bt2
  int jr = -1;
  for(int k = 2; k < M[i+1].child.size()- 1; k+=2) {
    if (j == M[i+1].child[k].neighbor_l()->array_index()) {
      int bt = M[i+1].child[k].btype;
      bcr[bt]++;
      if (bt > 0) { jr = k; }
    }
  }

  bool res = false; // default is to show dot

  // Case 1: no adjacent cells from left are boundaries, and no adjacent cells from right
  // are boundary cells, no adjacent cells from left are boundary cells, but (i,j) is a boundary cell
  // then we have a vertical boundary (or single point).
  if (bcl[1] + bcl[2] == 0 && bcr[1] + bcr[2] == 0 &&
      M[i][j].btype == M[i][j-1].btype && M[i][j].btype == M[i][j+1].btype &&
      M[i][j].truth == M[i][j-1].truth && M[i][j].truth == M[i][j+1].truth)
    res = true;

  else if (bcl[1] + bcl[2] == 1 && bcr[1] + bcr[2] == 1 &&
	   M[i][j].btype == M[i-1][jl].btype && M[i][j].btype == M[i+1][jr].btype &&
	   M[i][j].truth == M[i-1][jl].truth && M[i][j].truth == M[i+1][jr].truth)
    res = true;

 
  // cerr << "bcl = [" << bcl[1] << "," << bcl[2] << "], bcr = [" << bcr[1] << "," << bcr[2] << "] : res = "; 
  // cerr << res << endl;
  
  return res;
}


  

void ADDBOUNDARYINFO(Rend_Cell& M,QepcadCls &Q) {

  // Set 2D cells to boundary type zero
  for(int i = 1; i < M.child.size(); i+=2)
    for(int j = 1; j < M[i].child.size()-1; j += 2)
      M[i][j].btype = boundaryType(M,i,j);
  
  // Add boundaries for 1D cells (sections-over-sectors and sectors-over-sections)
  for(int i = 1; i < M.child.size(); i++)
    for(int j = 1 + (i % 2); j < M[i].child.size()-1; j += 2)
      M[i][j].btype = boundaryType(M,i,j);

  // Add boundaries for 0D cells (sections over sections)
  for(int i = 2; i < M.child.size(); i++)
    for(int j = 2; j < M[i].child.size()-1; j++) {
      M[i][j].btype = boundaryType(M,i,j);
      //cout << "(" << i << "," << j << ") : " << M[i][j].btype << endl;
    }
}
