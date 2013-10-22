#include "degree_flow.h"

#include <cstdio>
#include <vector>

#include "boost/assign/list_of.hpp"
#include "gtest/gtest.h"

using namespace boost::assign;
using namespace std;

void WriteToStderr(const char* s) {
  fprintf(stderr, s);
  fflush(stderr);
}

void CheckResult(const vector<vector<bool> >& expected_result,
                 vector<vector<bool> >& result) {
  /* for (size_t ii = 0; ii < result.size(); ++ii) {
    for (size_t jj = 0; jj < result[ii].size(); ++jj) {
      if (result[ii][jj]) {
        printf("1 ");
      } else {
        printf("0 ");
      }
    }
    printf("\n");
  }*/

  ASSERT_EQ(expected_result.size(), result.size());
  for (size_t ii = 0; ii < expected_result.size(); ++ii) {
    ASSERT_EQ(expected_result[ii].size(), result[ii].size());
    for (size_t jj = 0; jj < expected_result[ii].size(); ++jj) {
      EXPECT_EQ(expected_result[ii][jj], result[ii][jj])
          << "support mismatch at (" << ii << ", " << jj << ")";
    }
  }
}

void run_degree_flow(const vector<vector<double> >& x,
                     int k,
                     const vector<int>& row_degrees,
                     const vector<int>& col_degrees,
                     const vector<vector<bool> >& expected_result) {
  vector<vector<bool> > result;
  degree_flow(x, k, row_degrees, col_degrees, true, WriteToStderr, &result);
  CheckResult(expected_result, result);
}

TEST(DegreeFlowTest, SimpleSingleSelection) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(0));
  x.push_back(list_of(2)(3));

  int k = 1;
  vector<int> row_degrees = list_of(1)(1);
  vector<int> col_degrees = list_of(1)(0);

  vector<vector<bool> > result;
  result.push_back(list_of(0)(0));
  result.push_back(list_of(1)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, SimpleTwoElementSelection) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(0));
  x.push_back(list_of(2)(3));

  int k = 2;
  vector<int> row_degrees = list_of(1)(1);
  vector<int> col_degrees = list_of(2)(0);

  vector<vector<bool> > result;
  result.push_back(list_of(1)(0));
  result.push_back(list_of(1)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, SimpleDiagonalSelection) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(0));
  x.push_back(list_of(3)(3));

  int k = 2;
  vector<int> row_degrees = list_of(1)(1);
  vector<int> col_degrees = list_of(1)(1);

  vector<vector<bool> > result;
  result.push_back(list_of(1)(0));
  result.push_back(list_of(0)(1));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, ThreeElementSelection) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(3)(8));
  x.push_back(list_of(7)(0)(10));
  x.push_back(list_of(3)(2)(5));

  int k = 3;
  vector<int> row_degrees = list_of(1)(1)(1);
  vector<int> col_degrees = list_of(1)(1)(1);

  vector<vector<bool> > result;
  result.push_back(list_of(0)(0)(1));
  result.push_back(list_of(1)(0)(0));
  result.push_back(list_of(0)(1)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, FullSelection) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(3)(8));
  x.push_back(list_of(7)(1)(10));
  x.push_back(list_of(3)(2)(5));

  int k = 9;
  vector<int> row_degrees = list_of(3)(3)(3);
  vector<int> col_degrees = list_of(3)(3)(3);

  vector<vector<bool> > result;
  result.push_back(list_of(1)(1)(1));
  result.push_back(list_of(1)(1)(1));
  result.push_back(list_of(1)(1)(1));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, EmptySelectionK) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(3)(8));
  x.push_back(list_of(7)(1)(10));
  x.push_back(list_of(3)(2)(5));

  int k = 0;
  vector<int> row_degrees = list_of(3)(3)(3);
  vector<int> col_degrees = list_of(3)(3)(3);

  vector<vector<bool> > result;
  result.push_back(list_of(0)(0)(0));
  result.push_back(list_of(0)(0)(0));
  result.push_back(list_of(0)(0)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, EmptySelectionRowDegree) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(3)(8));
  x.push_back(list_of(7)(1)(10));
  x.push_back(list_of(3)(2)(5));

  int k = 9;
  vector<int> row_degrees = list_of(0)(0)(0);
  vector<int> col_degrees = list_of(3)(3)(3);

  vector<vector<bool> > result;
  result.push_back(list_of(0)(0)(0));
  result.push_back(list_of(0)(0)(0));
  result.push_back(list_of(0)(0)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, EmptySelectionColumnDegree) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(3)(8));
  x.push_back(list_of(7)(1)(10));
  x.push_back(list_of(3)(2)(5));

  int k = 9;
  vector<int> row_degrees = list_of(3)(3)(3);
  vector<int> col_degrees = list_of(0)(0)(0);

  vector<vector<bool> > result;
  result.push_back(list_of(0)(0)(0));
  result.push_back(list_of(0)(0)(0));
  result.push_back(list_of(0)(0)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, EmbeddedTwoDiagonal) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(3)(8)(4));
  x.push_back(list_of(7)(1)(10)(8));
  x.push_back(list_of(3)(2)(5)(0));
  x.push_back(list_of(9)(7)(2)(6));

  int k = 2;
  vector<int> row_degrees = list_of(1)(0)(1)(0);
  vector<int> col_degrees = list_of(1)(0)(1)(0);

  vector<vector<bool> > result;
  result.push_back(list_of(0)(0)(1)(0));
  result.push_back(list_of(0)(0)(0)(0));
  result.push_back(list_of(1)(0)(0)(0));
  result.push_back(list_of(0)(0)(0)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

TEST(DegreeFlowTest, LargeOneElementSelection) {
  vector<vector<double> > x;
  x.push_back(list_of(1)(3)(8)(4));
  x.push_back(list_of(7)(1)(10)(8));
  x.push_back(list_of(3)(2)(5)(0));
  x.push_back(list_of(9)(7)(2)(6));

  int k = 1;
  vector<int> row_degrees = list_of(0)(1)(0)(0);
  vector<int> col_degrees = list_of(0)(0)(1)(0);

  vector<vector<bool> > result;
  result.push_back(list_of(0)(0)(0)(0));
  result.push_back(list_of(0)(0)(1)(0));
  result.push_back(list_of(0)(0)(0)(0));
  result.push_back(list_of(0)(0)(0)(0));

  run_degree_flow(x, k, row_degrees, col_degrees, result);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
