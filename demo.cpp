//
// Demonstration program for hierarchical clustering
// with fastcluster by Daniel Muellner
//
// Line segments are clustered in two directions
//
// Author: Christoph Dalitz, 2018
//

#include <math.h>
#include <string.h>

#include <string>
#include <vector>
#include <stdio.h>
#include <algorithm>

#include "fastcluster.h"
#include "IO.h"
#include "gettime.h"

#define dim 128

// main program
int main(int argc, char** argv)
{
    timer t1;t1.start();
  int i,j,k,npoints;

  // parse command line
  std::string opt_infile;
  int opt_method = HCLUST_METHOD_SINGLE;
  const char* usagemsg = "Usage: hclust-demo <infile> [-m (single|complete|average|median)]\n";
  for (i=1; i<argc; i++) {
    if (0 == strcmp(argv[i], "-m")) {
      i++;
      if (i<argc) {
        if (0 == strcmp(argv[i], "single"))
            opt_method = HCLUST_METHOD_SINGLE;
        else if (0 == strcmp(argv[i], "complete"))
            opt_method = HCLUST_METHOD_COMPLETE;
        else if (0 == strcmp(argv[i], "average"))
            opt_method = HCLUST_METHOD_AVERAGE;
        else if (0 == strcmp(argv[i], "median"))
            opt_method = HCLUST_METHOD_MEDIAN;
        else {
          fputs(usagemsg, stderr);
          return 1;
        }
      } else {
        fputs(usagemsg, stderr);
        return 1;
      }
    }
    else if (argv[i][0] == '-') {
      fputs(usagemsg, stderr);
      return 1;
    }
    else {
      opt_infile = argv[i];
    }
  }
  if (opt_infile == "") {
    fputs(usagemsg, stderr);
    return 1;
  }
  

  _seq<point<dim> > PIn = readPointsFromFileNoHeader<point<dim>>(&opt_infile[0]);
    npoints = PIn.n;
    std::cout << npoints << std::endl;

    // computation of condensed distance matrix
    double* distmat = new double[(npoints*(npoints-1))/2];
    k = 0;
    for (i=0; i<npoints; i++) {
        for (j=i+1; j<npoints; j++) {
            distmat[k] = PIn.A[i].pointDist(PIn.A[j]);
            k++;
        }
    }
    free(PIn.A);
    std::cout << "matrix " <<  t1.next() << std::endl;
    
    // clustering call
    int* merge = new int[2*(npoints-1)];
    double* height = new double[npoints-1];
    hclust_fast(npoints, distmat, opt_method, merge, height);
    std::cout << "hierarchy " <<  t1.next() << std::endl;
    
    int* labels = new int[npoints];
    cutree_k(npoints, merge, 2, labels);
    std::cout << "cut " <<  t1.next() << std::endl;
    //cutree_cdist(npoints, merge, height, 0.5, labels);
    
    // clean up
    delete[] distmat;
    delete[] merge;
    delete[] height;
    delete[] labels;
    
  return 0;
}
