//computeRHS() method for MatrixFreePDE class

#include "../../include/matrixFreePDE.h"
#include "../../include/variableContainer.h"

//update RHS of each field
template <int dim, int degree>
void MatrixFreePDE<dim,degree>::computeRHS(){
  //log time
  computing_timer.enter_section("matrixFreePDE: computeRHS");

  //clear residual vectors before update
  for(unsigned int fieldIndex=0; fieldIndex<fields.size(); fieldIndex++){
    (*residualSet[fieldIndex])=0.0;
  }

  //call to integrate and assemble
  matrixFreeObject.cell_loop (&MatrixFreePDE<dim,degree>::getRHS, this, residualSet, solutionSet);

  //end log
  computing_timer.exit_section("matrixFreePDE: computeRHS");
}

template <int dim, int degree>
void MatrixFreePDE<dim,degree>::getRHS(const MatrixFree<dim,double> &data,
                                        std::vector<vectorType*> &dst,
                                        const std::vector<vectorType*> &src,
                                        const std::pair<unsigned int,unsigned int> &cell_range) const{

    variableContainer<dim,degree,dealii::VectorizedArray<double> > variable_list(data,userInputs.varInfoListRHS);

    //loop over cells
    for (unsigned int cell=cell_range.first; cell<cell_range.second; ++cell){

        // Initialize, read DOFs, and set evaulation flags for each variable
        variable_list.reinit_and_eval(src, cell, userInputs.need_value, userInputs.need_gradient, userInputs.need_hessian);

        unsigned int num_q_points = variable_list.get_num_q_points();

        //loop over quadrature points
        for (unsigned int q=0; q<num_q_points; ++q){
            variable_list.q_point = q;

            dealii::Point<dim, dealii::VectorizedArray<double> > q_point_loc = variable_list.get_q_point_location();

            // Calculate the residuals
            residualRHS(variable_list,q_point_loc);

        }

        variable_list.integrate_and_distribute(dst, userInputs.value_residual, userInputs.gradient_residual);
    }
}

    #include "../../include/matrixFreePDE_template_instantiations.h"
