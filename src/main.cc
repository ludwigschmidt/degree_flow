#include <vector>
#include <cstdio>
#include <cmath>

#include "degree_flow.h"

using namespace std;

int r, c;
int k;
vector<int> row_degrees;
vector<int> col_degrees;
vector<vector<double> > a;
vector<vector<double> > support;

void output_function(const char* s) {
  fprintf(stderr, s);
  fflush(stderr);
}

int main() {
  scanf("%d %d %d", &r, &c, &k);
  row_degrees.resize(r);
  for (int ii = 0; ii < r; ++ii) {
    scanf("%d", &row_degrees[ii]);
  }
  col_degrees.resize(c);
  for (int jj = 0; jj < c; ++jj) {
    scanf("%d", &col_degrees[jj]);
  }

  a.resize(r);
  for (int ii = 0; ii < r; ++ii) {
    a[ii].resize(c);
    for (int jj = 0; jj < c; ++jj) {
      scanf("%lf", &(a[ii][jj]));
    }
  }

  vector<vector<bool> > result;
  degree_flow(a, k, row_degrees, col_degrees, true, output_function, &result);

  for (int ii = 0; ii < r; ++ii) {
    for (int jj = 0; jj < c; ++jj) {
      if (result[ii][jj]) {
        printf("1 ");
      } else {
        printf("0 ");
      }
    }
    printf("\n");
  }

  return 0;
}
