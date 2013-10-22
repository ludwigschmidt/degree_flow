#ifndef __DEGREE_FLOW_H__
#define __DEGREE_FLOW_H__

#include <vector>

void degree_flow(
    // signal coefficients (will not be squared)
    const std::vector<std::vector<double> >& x,
    // Total sparsity
    int k,
    // Row degrees
    const std::vector<int>& row_degrees,
    // Column degrees
    const std::vector<int>& col_degrees,
    // Verbose output?
    bool verbose,
    // The output function
    void (*output_function)(const char*),
    // Result: a bool matrix indicating support
    std::vector<std::vector<bool> >* result);

#endif
