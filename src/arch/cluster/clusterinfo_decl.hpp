#ifndef _CLUSTER_INFO
#define _CLUSTER_INFO

#include <iostream>
#include "config_decl.hpp"
#include "system_decl.hpp"

namespace nanos {
   namespace ext {

      class ClusterInfo
      {
         private:
            static unsigned int _numSegments;
            static void ** _segmentAddrList;
            static std::size_t * _segmentLenList;
            static unsigned int _extraPEsCount;
            static std::string _conduit;
            static std::size_t _nodeMem;
            static int _gpuPresend;
            static int _smpPresend;
            static System::CachePolicyType _cachePolicy;

         public:
            ClusterInfo() {}

            ~ClusterInfo()
            {
               if ( _segmentAddrList != NULL )
                  delete _segmentAddrList;

               if ( _segmentLenList != NULL )
                  delete _segmentLenList;
            }

            static void prepare( Config& cfg );
            static void addSegments( unsigned int numSegments, void **segmentAddr, size_t *segmentSize );
            static void * getSegmentAddr( unsigned int idx );
            static std::size_t getSegmentLen( unsigned int idx );
            static unsigned int getExtraPEsCount();
            static void setExtraPEsCount( unsigned int num );
            static std::size_t getNodeMem();
            static int getGpuPresend();
            static int getSmpPresend();
            //static const std::string & getNetworkConduit();
            static void setUpCache();
            static System::CachePolicyType getCachePolicy ( void );
      };
   }
}

#endif
