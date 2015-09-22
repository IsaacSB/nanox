/*************************************************************************************/
/*      Copyright 2015 Barcelona Supercomputing Center                               */
/*                                                                                   */
/*      This file is part of the NANOS++ library.                                    */
/*                                                                                   */
/*      NANOS++ is free software: you can redistribute it and/or modify              */
/*      it under the terms of the GNU Lesser General Public License as published by  */
/*      the Free Software Foundation, either version 3 of the License, or            */
/*      (at your option) any later version.                                          */
/*                                                                                   */
/*      NANOS++ is distributed in the hope that it will be useful,                   */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/*      GNU Lesser General Public License for more details.                          */
/*                                                                                   */
/*      You should have received a copy of the GNU Lesser General Public License     */
/*      along with NANOS++.  If not, see <http://www.gnu.org/licenses/>.             */
/*************************************************************************************/

/*
<testinfo>
test_generator=gens/opencl-generator
test_generator_ENV='test_architecture=smp'
test_schedule=bf
</testinfo>
*/

#include "assert.h"
#include <iostream>
#include "openclprocessor.hpp"

using namespace std;
using namespace nanos;
using namespace ext;

int main()
{
  // Class to check
  ::OpenCLAdapter *openCLAdapter;
  openCLAdapter = new ::OpenCLAdapter();

  cl_kernel kernel1 = (cl_kernel) 1111;
  cl_kernel kernel2 = (cl_kernel) 2222;

  ::Execution *execution1, *execution2;
  execution1 = new ::Execution(3,1,2,3,10);
  execution2 = new ::Execution(3,1,2,3,5);

  std::map<cl_kernel,OpenCLAdapter::DimsBest> bestExec;
  std::map<cl_kernel,OpenCLAdapter::DimsExecutions> nExec;

  ::Dims dims1(3,2,4,8,1.0);
  ::Dims dims2(3,8,4,2,1.0);

  ::OpenCLAdapter::DimsBest dims1Best;

  /* Begin testing */

  assert(openCLAdapter->getBestExec().size() == 0);
  // Several kernels
  openCLAdapter->updateProfiling(kernel1, execution1/* 10 */, dims1); // Add one execution
  assert(openCLAdapter->getBestExec().size() == 1);
  bestExec = openCLAdapter->getBestExec();
  nExec = openCLAdapter->getExecutions();
  dims1Best = bestExec[kernel1];
  assert(dims1Best.size() == 1);
  assert(dims1Best[dims1]->getTime() == 10);

  openCLAdapter->updateProfiling(kernel2, execution2/* 5 */, dims2); // Add one execution
  assert(openCLAdapter->getBestExec().size() == 2);
  bestExec = openCLAdapter->getBestExec();
  dims1Best = bestExec[kernel2];
  assert(dims1Best[dims2]->getTime() == 5);

  /* End testing */

  delete execution1;
  delete execution2;

  return 0;
}
