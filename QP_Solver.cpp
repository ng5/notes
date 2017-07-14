// QP_Solver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <CGAL/basic.h>
#include <CGAL/Quotient.h>
#include <CGAL/QP_models.h>
#include <CGAL/QP_functions.h>
#include <CGAL/MP_Float.h>
typedef CGAL::MP_Float ET;

typedef CGAL::Quadratic_program_from_iterators
<double**, // for A
 double*, // for b
 CGAL::Const_oneset_iterator<CGAL::Comparison_result>, // for r
 bool*, // for fl
 double*, // for l
 bool*, // for fu
 double*, // for u
 double**, // for D
 double*> // for c 
Program;
typedef CGAL::Quadratic_program_solution<ET> Solution;

int main(){
	double Ax[] = {1, -1}; // column for x
	double Ay[] = {1, 3}; // column for y
	double* A[] = {Ax, Ay}; // A comes columnwise
	double b[] = {7, 4}; // right-hand side
	CGAL::Const_oneset_iterator<CGAL::Comparison_result>
		r(CGAL::SMALLER); // constraints are "<="
	bool fl[] = {true, true}; // both x, y are lower-bounded
	double l[] = {0, 0};
	bool fu[] = {false, true}; // only y is upper-bounded
	double u[] = {0, 4}; // x's u-entry is ignored
	double D1[] = {2}; // 2D_{1,1}
	double D2[] = {0, 8}; // 2D_{2,1}, 2D_{2,2}
	double* D[] = {D1, D2}; // D-entries on/below diagonal
	double c[] = {0, -32};
	double c0 = 64; // constant term
	Program qp(2, 2, A, b, r, fl, l, fu, u, D, c, c0);
	// solve the program, using ET as the exact type
	Solution s = CGAL::solve_quadratic_program(qp, ET());
	// output solution
	std::cout << "Number of iterations = " << s.number_of_iterations() << std::endl;
	for (auto x = s.variable_values_begin(); x != s.variable_values_end(); ++x){
		std::cout << "value=" << CGAL::to_double(*x) << std::endl;
	}
	return 0;
}
