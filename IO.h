#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include "sequence.h"

using namespace std;

  // A structure that keeps a sequence of strings all allocated from
  // the same block of memory
  struct words {
    long n; // total number of characters
    char* Chars;  // array storing all strings
    long m; // number of substrings
    char** Strings; // pointers to strings (all should be null terminated)
    words() {}
    words(char* C, long nn, char** S, long mm)
      : n(nn), Chars(C), m(mm), Strings(S) {}
    void del() {free(Chars); free(Strings);}
  };

template <int _dim> class point {
public:
  typedef double floatT;
//   typedef vect<_dim> vectT;
  typedef point pointT;
  floatT x[_dim];
  static const int dim = _dim;
  static constexpr double empty = numeric_limits<double>::max();
  int dimension() {return _dim;}
  bool isEmpty() {return x[0]==empty;}
  point() { for (int i=0; i<_dim; ++i) x[i]=empty; }
//   point(vectT v) { for (int i=0; i<_dim; ++i) x[i]=v[i]; }
  point(floatT* p) { for (int i=0; i<_dim; ++i) x[i]=p[i]; }
  point(pointT* p) { for (int i=0; i<_dim; ++i) x[i]=p->x[i]; }
  void print() {
    cout << std::setprecision(2);
    cout << ":(";
    for (int i=0; i<_dim-1; ++i) cout << x[i] << ",";
    cout << x[_dim-1] << "):";
  }
//   pointT operator+(vectT op2) {
//     floatT xx[_dim];
//     for (int i=0; i<_dim; ++i) xx[i] = x[i]+op2.x[i];
//     return pointT(xx);
//   }
//   vectT operator-(pointT op2) {
//     floatT xx[_dim];
//     for (int i=0; i<_dim; ++i) xx[i] = x[i]-op2.x[i];
//     return pointT(xx);
//   }
  floatT operator[](int i) {return x[i];}
  void updateX(int i, floatT val) {x[i]=val;}
  void minCoords(pointT b) {
    for (int i=0; i<_dim; ++i) x[i] = min(x[i], b.x[i]);}
  void minCoords(floatT* b) {
    for (int i=0; i<_dim; ++i) x[i] = min(x[i], b[i]);}
  void maxCoords(pointT b) {
    for (int i=0; i<_dim; ++i) x[i] = max(x[i], b.x[i]);}
  void maxCoords(floatT* b) {
    for (int i=0; i<_dim; ++i) x[i] = max(x[i], b[i]);}
  intT quadrant(pointT center) {
    intT index = 0;
    intT offset = 1;
    for (int i=0; i<_dim; ++i) {
      if (x[i] > center.x[i]) index += offset;
      offset *= 2;
    }
    return index;
  }
  bool outOfBox(pointT center, floatT hsize) {
    for (int i=0; i<_dim; ++i) {
      if (x[i]-hsize > center.x[i] || x[i]+hsize < center.x[i])
        return true;
    }
    return false;
  }
  floatT pointDist(pointT p) {
    floatT xx=0;
    for (int i=0; i<_dim; ++i) xx += (x[i]-p.x[i])*(x[i]-p.x[i]);
    return sqrt(xx);
  }
};

 
  inline bool isSpace(char c) {
    switch (c)  {
    case '\r': 
    case '\t': 
    case '\n': 
    case 0:
    case ' ' : return true;
    default : return false;
    }
  }

  inline bool isComma(char c) {
    switch (c)  {
    case '\r': 
    case '\t': 
    case '\n': 
    case ',': 
    case 0:
    case ' ' : return true;
    default : return false;
    }
  }

  _seq<char> readStringFromFile(char *fileName) {
    ifstream file (fileName, ios::in | ios::binary | ios::ate);
    if (!file.is_open()) {
      std::cout << "Unable to open file: " << fileName << std::endl;
      abort();
    }
    long end = file.tellg();
    file.seekg (0, ios::beg);
    long n = end - file.tellg();
    // initializes in parallel
    //char* bytes = newArray(n+1, (char) 0);
    char* bytes = newA(char, n+1);
    for (long i = 0; i < n+1; i++) bytes[i] = 0; //parallel_for
    file.read (bytes,n);
    file.close();
    return _seq<char>(bytes,n);
  }
  
    // parallel code for converting a string to words
  words stringToWords(char *Str, long n) {
    for (long i=0; i < n; i++)  //parallel_
      if (isSpace(Str[i])) Str[i] = 0; 

    // mark start of words
    bool *FL = newA(bool,n);
    FL[0] = Str[0];
    for (long i=1; i < n; i++) FL[i] = Str[i] && !Str[i-1]; //parallel_
    
    // offset for each start of word
    _seq<long> Off = sequence::packIndex<long>(FL, n);
    long m = Off.n;
    long *offsets = Off.A;

    // pointer to each start of word
    char **SA = newA(char*, m);
    for (long j=0; j < m; j++) SA[j] = Str+offsets[j]; //parallel_

    free(offsets); free(FL);
    return words(Str,n,SA,m);
  }

  template <class pointT>
  void parsePoints(char** Str, pointT* P, intT n) {
    int d = pointT::dim;
    double* a = newA(double,n*d);
    {parallel_for (long i=0; i < d*n; i++) 
  a[i] = atof(Str[i]);}
    {parallel_for (long i=0; i < n; i++) 
  P[i] = pointT(a+(d*i));}
    free(a);
  }

  template <class pointT>
  _seq<pointT> readPointsFromFileNoHeader(char* fname) {
    _seq<char> S = readStringFromFile(fname);
    words W = stringToWords(S.A, S.n);
    int d = pointT::dim;
    long n = (W.m)/d;
    pointT *P = newA(pointT, n);
    parsePoints(W.Strings, P, n);
    return _seq<pointT>(P, n);
  }
