#include <vector>
#include <string>
#include <set>

#include <math.h>
#include <matrix.h>
#include <mex.h>

#include "mex_helper.h"
#include "degree_flow.h"

using namespace std;

void output_function(const char* s) {
  mexPrintf(s);
  mexEvalString("drawnow;");
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if (nrhs < 4) {
    mexErrMsgTxt("At least four input argument required (amplitudes, sparsity,"
        " row degree, column degrees).");
  }
  if (nrhs > 5) {
    mexErrMsgTxt("Too many input arguments, at most five: amplitudes, sparsity,"
        " row degree, column degrees and the options struct.");
  }
  if (nlhs > 1) {
    mexErrMsgTxt("Too many output arguments.");
  }

  vector<vector<double> > a;
  if (!get_double_matrix(prhs[0], &a)) {
    mexErrMsgTxt("Amplitudes need to be a two-dimensional double array.");
  }
  if (a.size() == 0) {
    mexErrMsgTxt("The input signal must have at least one row.");
  }

  int k = 0;
  if (!get_double_as_int(prhs[1], &k)) {
    mexErrMsgTxt("Sparsity has to be a double scalar.");
  }
  
  vector<int> row_degrees;
  if (!get_double_row_vector_as_ints(prhs[2], &row_degrees)) {
    mexErrMsgTxt("Row degrees has to be a double row vector.");
  }
  if (row_degrees.size() != a.size()) {
    mexErrMsgTxt("The row degree vector must have as many entries as X has "
                 "rows.");
  }

  vector<int> col_degrees;
  if (!get_double_row_vector_as_ints(prhs[3], &col_degrees)) {
    mexErrMsgTxt("Col degrees has to be a double row vector.");
  }
  if (col_degrees.size() != a[0].size()) {
    mexErrMsgTxt("The column degree vector must have as many entries as X has "
                 "columns.");
  }
  
  bool verbose = false;
  if (nrhs == 5) {
    set<string> known_options;
    known_options.insert("verbose");
    vector<string> options;
    if (!get_fields(prhs[3], &options)) {
      mexErrMsgTxt("Cannot get fields from options argument.");
    }
    for (size_t ii = 0; ii < options.size(); ++ii) {
      if (known_options.find(options[ii]) == known_options.end()) {
        const size_t tmp_size = 1000;
        char tmp[tmp_size];
        snprintf(tmp, tmp_size, "Unknown option \"%s\"\n", options[ii].c_str());
        mexErrMsgTxt(tmp);
      }
    }

    if (has_field(prhs[3], "verbose")
        && !get_bool_field(prhs[3], "verbose", &verbose)) {
      mexErrMsgTxt("verbose flag has to be a boolean scalar.");
    }
  }
  
  vector<vector<bool> > support;
  degree_flow(a, k, row_degrees, col_degrees, verbose, output_function,
              &support);
  if (nlhs >= 1) {
    set_double_matrix(&(plhs[0]), support);
  }
}
