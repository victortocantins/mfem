
#include "conv_rates.hpp"

ConvergenceRates::ConvergenceRates()
{
#ifdef MFEM_USE_MPI
   comm_flag = 0;
#endif
   print_flag = 1;
}

#ifdef MFEM_USE_MPI
ConvergenceRates::ConvergenceRates(MPI_Comm _comm) : comm(_comm)
{
   comm_flag = 1;
   int rank;
   MPI_Comm_rank(comm, &rank);
   if (rank==0) 
   {
      print_flag = 1;
   }
}
#endif

void ConvergenceRates::Clear()
{
   counter=0;
   L2Rates.SetSize(0);
}

void ConvergenceRates::RegisterSolution(GridFunction * gf, Coefficient * u)
{
   int tdofs = gf->FESpace()->GetTrueVSize();
#ifdef MFEM_USE_MPI   
   if (comm_flag)
   {
      MPI_Allreduce(&tdofs,&tdofs,1,MPI_INT,MPI_SUM,comm);
   }
#endif   
   ndofs.Append(tdofs);

   double L2Err = gf->ComputeL2Error(*u);
   L2Errors.Append(L2Err);
   double val = (counter) ? log(L2Errors[counter-1]/L2Err)/log(2.0) : 0.0;
   L2Rates.Append(val);

   ConstantCoefficient zero(0.0);
   double L2norm = gf->ComputeL2Error(zero);
   L2RelErrors.Append(L2Err/L2norm);
   double rval = (counter) ? log(L2RelErrors[counter-1]/(L2Err/L2norm))/log(2.0) : 0.0;
   L2RelRates.Append(rval);
   counter++;
}
void ConvergenceRates::RegisterSolution(GridFunction * gf, VectorCoefficient * U)
{
   int tdofs = gf->FESpace()->GetTrueVSize();
#ifdef MFEM_USE_MPI   
   if (comm_flag)
   {
      MPI_Allreduce(&tdofs,&tdofs,1,MPI_INT,MPI_SUM,comm);
   }
#endif   
   ndofs.Append(tdofs);

   double L2Err = gf->ComputeL2Error(*U);
   L2Errors.Append(L2Err);
   double val = (counter) ? log(L2Errors[counter-1]/L2Err)/log(2.0) : 0.0;
   L2Rates.Append(val);
   
   Vector vec(U->GetVDim()); vec = 0.0;
   VectorConstantCoefficient zero(vec);
   double L2norm = gf->ComputeL2Error(zero);
   L2RelErrors.Append(L2Err);
   double rval = (counter) ? log(L2RelErrors[counter-1]/(L2Err/L2norm))/log(2.0) : 0.0;
   L2RelRates.Append(rval);
   counter++;
}
double ConvergenceRates::GetL2Error(int n)
{
   MFEM_VERIFY(n<= counter,"Step out of bounds")
   return L2Errors[n];
}

Array<double> * ConvergenceRates::GetRates()
{
   return &L2Rates;
}

Array<double> * ConvergenceRates::GetL2Errors()
{
   return &L2Errors;
}

void ConvergenceRates::Print()
{
   if (print_flag)
   {
      cout << " ------------------------------------------------------------------------"
           << endl;
      cout  << right<< setw(11)<< "DOFs "<< setw(15) << "L^2 error "<< setw(15);
      cout << "L^2 rate " << setw(15) << "L^2 Rel error "<< setw(15) << "L^2 Rel rate " << endl;
      cout << " ------------------------------------------------------------------------"
           << endl;
      cout << setprecision(4);
      for (int i =0; i<counter; i++)
      {
         cout << right << setw(10)<< ndofs[i] << setw(16) 
              << scientific << L2Errors[i] << setw(13)  << fixed << L2Rates[i] << setw(15) 
              << scientific << L2RelErrors[i] << setw(13)  << fixed << L2RelRates[i] << endl;
      }
   }
}
