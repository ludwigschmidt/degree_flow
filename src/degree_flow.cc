#include "degree_flow.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <limits>
#include <queue>
#include <vector>

using namespace std;

const int kOutputBufferSize = 10000;
char output_buffer[kOutputBufferSize];

typedef size_t NodeIndex;
typedef size_t EdgeIndex;

struct Edge {
  NodeIndex to;
  int capacity;
  double cost;
  EdgeIndex opposite;

  Edge(NodeIndex _to, int _capacity, double _cost, EdgeIndex _opposite)
    : to(_to), capacity(_capacity), cost(_cost), opposite(_opposite) { }
};

size_t num_nodes;
size_t num_connected_nodes;
size_t num_rows;
size_t num_cols;
// source, sing
NodeIndex s, t;
// edges leaving a node
vector<vector<EdgeIndex> > outgoing_edges;
// set of all edges
vector<Edge> e;

// node potentials
vector<double> potential;

long long total_inner_iterations;
long long checking_inner_iterations;
long long updating_inner_iterations;

EdgeIndex EntryEdgeIndex(int r, int c) {
  return 2 * (num_cols * r + c);
}

NodeIndex RowNodeIndex(int r) {
  return 2 + r;
}

NodeIndex ColNodeIndex(int c) {
  return 2 + num_rows + c;
}

void BuildGraph(const vector<vector<double> >& x,
                const vector<int>& row_degrees,
                const vector<int>& col_degrees);
void ComputeInitialPotentials();
bool FindPath();

void degree_flow(
    // signal coefficients (will not be squared)
    const vector<vector<double> >& x,
    // Total sparsity
    int k,
    // Row degrees
    const vector<int>& row_degrees,
    // Column degrees
    const vector<int>& col_degrees,
    // Verbose output?
    bool verbose,
    // The output function
    void (*output_function)(const char*),
    // Result: a bool matrix indicating support
    vector<vector<bool> >* result) {

  clock_t total_time_begin = clock();

  num_rows = x.size();
  if (num_rows == 0) {
    snprintf(output_buffer, kOutputBufferSize, "Signal must have at least one "
             "row.");
    output_function(output_buffer);
    result->clear();
    return;
  }

  num_cols = x[0].size();
  if (num_cols == 0) {
    snprintf(output_buffer, kOutputBufferSize, "Signal must have at least one "
             "column.");
    output_function(output_buffer);
    result->clear();
    return;
  }

  for (size_t row = 1; row < num_rows; ++row) {
    if (x[row].size() != num_cols) {
      snprintf(output_buffer, kOutputBufferSize, "All columns must have the "
               "same size.");
      output_function(output_buffer);
      result->clear();
      return;
    }
  }

  if (verbose) {
    snprintf(output_buffer, kOutputBufferSize, "r = %zd,  c = %zd,  k = %d\n",
        num_rows, num_cols, k);
    output_function(output_buffer);
  }

  clock_t graph_construction_time_begin = clock();
  
  BuildGraph(x, row_degrees, col_degrees);
  ComputeInitialPotentials();

  clock_t graph_construction_time = clock() - graph_construction_time_begin;
  if (verbose) {
    snprintf(output_buffer, kOutputBufferSize, "The graph has %zd nodes and %zd"
        " edges.\n", num_nodes, e.size());
    output_function(output_buffer);
    snprintf(output_buffer, kOutputBufferSize, "Total construction time: %f "
        "s\n", static_cast<double>(graph_construction_time) / CLOCKS_PER_SEC);
    output_function(output_buffer);
  }

  for (int ii = 0; ii < k; ++ii) {
    if (!FindPath()) {
      snprintf(output_buffer, kOutputBufferSize, "Could not fit %d nonzeros "
               "into the matrix, the support has %d nonzeros.\n", k, ii);
      output_function(output_buffer);
      break;
    }

    const double threshold_step = 0.1;
    double threshold = threshold_step;
    if (verbose) {
      if (k <= 10) {
        snprintf(output_buffer, kOutputBufferSize, "%d entries selected\n",
                 ii + 1);
        output_function(output_buffer);
      } else {
        double fraction = static_cast<double>(ii + 1) / k;
        if (fraction >= threshold) {
          threshold += threshold_step;
          snprintf(output_buffer, kOutputBufferSize, "%d entries selected "
                   "(%.2lf%%)\n", ii + 1, 100 * fraction);
          output_function(output_buffer);
        }
      }
    }
  }
 
  vector<vector<bool> >& resultref = *result;
  if (x.size() != resultref.size()) {
    resultref.resize(x.size());
  }

  for (size_t ii = 0; ii < x.size(); ++ii) {
    if (x[ii].size() != resultref[ii].size()) {
      resultref[ii].resize(x[ii].size());
    }
    for (size_t jj = 0; jj < x[ii].size(); ++jj) {
      resultref[ii][jj] = (e[EntryEdgeIndex(ii, jj)].capacity == 0);
    }
  }

  clock_t total_time = clock() - total_time_begin;
  if (verbose) {
    snprintf(output_buffer, kOutputBufferSize, "Total time %lf s\n",
        static_cast<double>(total_time) / CLOCKS_PER_SEC);
    output_function(output_buffer);

    snprintf(output_buffer, kOutputBufferSize, "Performance diagnostics:\n"
             "Total inner iterations: %lld\n"
             "Checking inner iterations: %lld\n"
             "Updating inner iterations: %lld\n",
             total_inner_iterations, checking_inner_iterations,
             updating_inner_iterations);
    output_function(output_buffer);
  }
}


void BuildGraph(const vector<vector<double> >& x,
                const vector<int>& row_degrees,
                const vector<int>& col_degrees) {
  e.clear();
  outgoing_edges.clear();

  s = 0;
  t = 1;
  num_nodes = num_rows + num_cols + 2;
  num_connected_nodes = num_nodes;
  outgoing_edges.resize(num_nodes);

  // connections between rows and columns
  for (size_t row = 0; row < num_rows; ++row) {
    for (size_t col = 0; col < num_cols; ++col) {
      EdgeIndex next_edge_index = e.size();
      Edge forward(ColNodeIndex(col), 1, -abs(x[row][col]),
                   next_edge_index + 1);
      Edge backward(RowNodeIndex(row), 0, abs(x[row][col]),
                    next_edge_index);
      e.push_back(forward);
      e.push_back(backward);
      // TODO: this approach is not ideal because we potentially have many
      // useless edges in e.
      if (row_degrees[row] > 0) {
        outgoing_edges[RowNodeIndex(row)].push_back(next_edge_index);
        outgoing_edges[ColNodeIndex(col)].push_back(next_edge_index + 1);
      }
    }
  }

  // connections from source to rows
  for (size_t row = 0; row < num_rows; ++row) {
      EdgeIndex next_edge_index = e.size();
      Edge forward(RowNodeIndex(row), row_degrees[row], 0.0,
                   next_edge_index + 1);
      Edge backward(s, 0, 0.0, next_edge_index);
      e.push_back(forward);
      e.push_back(backward);
      if (row_degrees[row] > 0) {
        outgoing_edges[s].push_back(next_edge_index);
        outgoing_edges[RowNodeIndex(row)].push_back(next_edge_index + 1);
      } else {
        num_connected_nodes -= 1;
      }
  }

  // connections from columns to sink
  for (size_t col = 0; col < num_cols; ++col) {
      EdgeIndex next_edge_index = e.size();
      Edge forward(t, col_degrees[col], 0.0, next_edge_index + 1);
      Edge backward(ColNodeIndex(col), 0, 0.0, next_edge_index);
      e.push_back(forward);
      e.push_back(backward);
      outgoing_edges[ColNodeIndex(col)].push_back(next_edge_index);
      outgoing_edges[t].push_back(next_edge_index + 1);
  }
}

void ComputeInitialPotentials() {
  potential.clear();
  // Initialize
  potential.resize(num_nodes, numeric_limits<double>::infinity());

  // Source and row nodes have potential 0
  potential[s] = 0.0;
  for (size_t row = 0; row < num_rows; ++row) {
    potential[RowNodeIndex(row)] = 0.0;
  }

  // Column nodes
  for (size_t row = 0; row < num_rows; ++row) {
    for (size_t iedge = 0; iedge < outgoing_edges[RowNodeIndex(row)].size();
         ++iedge) {
      const Edge& cur_edge = e[outgoing_edges[RowNodeIndex(row)][iedge]];
      potential[cur_edge.to] = min(potential[cur_edge.to], cur_edge.cost);
    }
  }

  // Sink
  for (size_t col = 0; col < num_cols; ++col) {
    potential[t] = min(potential[t], potential[ColNodeIndex(col)]);
  }
}

vector<bool> visited;
vector<double> dst;
vector<EdgeIndex> edge_taken_to;

bool FindPath() {
  typedef pair<double, NodeIndex> q_elem;
  
  if (visited.size() != num_nodes) {
    visited.resize(num_nodes);
  }
  fill(visited.begin(), visited.end(), false);

  if (dst.size() != num_nodes) {
    dst.resize(num_nodes, numeric_limits<double>::infinity());
  }
  fill(dst.begin(), dst.end(), numeric_limits<double>::infinity());

  if (edge_taken_to.size() != num_nodes) {
    edge_taken_to.resize(num_nodes, s);
  }
  
  priority_queue<q_elem> q;

  dst[s] = 0.0;
  q.push(q_elem(-dst[s], s));

  size_t num_found = 0;
  
  while (!q.empty() && num_found < num_connected_nodes) {
    q_elem top = q.top();
    q.pop();

    if (visited[top.second]) {
      continue;
    }

    NodeIndex cur_node = top.second;
    visited[cur_node] = true;
    ++num_found;

    NodeIndex next_node;
    for (vector<EdgeIndex>::iterator iter = outgoing_edges[cur_node].begin();
         iter != outgoing_edges[cur_node].end(); ++iter) {
      const Edge& cur_e = e[*iter];
      next_node = cur_e.to;

      ++total_inner_iterations;

      if (cur_e.capacity == 0) {
        continue;
      }
      if (visited[next_node]) {
        continue;
      }

      ++checking_inner_iterations;

      double adjusted_edge_cost = cur_e.cost + potential[cur_node]
                                             - potential[next_node];
      if (dst[cur_node] + adjusted_edge_cost < dst[next_node]) {
        dst[next_node] = dst[cur_node] + adjusted_edge_cost;
        q.push(q_elem(-dst[next_node], next_node));
        edge_taken_to[next_node] = *iter;

        ++updating_inner_iterations;
      }
    }
  }
  
  if (num_found < num_connected_nodes) {
    //printf("%d vs %d\n", num_found, num_connected_nodes);
    return false;
  }

  // change potentials
  for (size_t ii = 0; ii < potential.size(); ++ii) {
    potential[ii] += dst[ii];
  }

  // change capacities
  NodeIndex cur_node = t;
  do {
    Edge& forward_edge = e[edge_taken_to[cur_node]];
    forward_edge.capacity -= 1;
    e[forward_edge.opposite].capacity += 1;
    //printf("Decreasing capacity of %zu, increasing of %zu\n",
    //       edge_taken_to[cur_node], forward_edge.opposite);
    cur_node = e[forward_edge.opposite].to;
  } while (cur_node != s);

  return true;
}
