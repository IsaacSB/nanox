/*************************************************************************************/
/*      Copyright 2009 Barcelona Supercomputing Center                               */
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

#include "nanos.h"
#include "nanos-gpu.h"
#include "basethread.hpp"
#include "gpudd.hpp"
#include "gpuprocessor.hpp"

using namespace nanos;


const size_t nanos_gpu_dd_size = sizeof(ext::GPUDD);

NANOS_API_DEF(void *, nanos_gpu_factory, ( void *prealloc, void *args ))
{
   nanos_smp_args_t *smp = ( nanos_smp_args_t * ) args;
   if ( prealloc != NULL )
   {
      return ( void * )new (prealloc) ext::GPUDD( smp->outline );
   }
   else
   {
      return ( void * ) new ext::GPUDD( smp->outline );
   }
}


NANOS_API_DEF( cudaStream_t, nanos_get_kernel_execution_stream, ( void ) )
{
   return ( ( nanos::ext::GPUProcessor *) getMyThreadSafe()->runningOn() )->getGPUProcessorInfo()->getKernelExecStream();
}

NANOS_API_DEF( cublasHandle_t, nanos_get_cublas_handle, ( void ) )
{
   return ( cublasHandle_t ) ( ( nanos::ext::GPUThread *)getMyThreadSafe() )->getCUBLASHandle();
}

NANOS_API_DEF(void *, nanos_malloc_pinned_cuda, ( size_t size ))
{
   return sys.getPinnedAllocatorCUDA().allocate( size );
}

NANOS_API_DEF( void, nanos_free_pinned_cuda, ( void * address ) )
{
   return sys.getPinnedAllocatorCUDA().free( address );
}

