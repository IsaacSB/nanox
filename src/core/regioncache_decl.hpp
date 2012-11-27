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

#ifndef _NANOS_REGION_CACHE_H
#define _NANOS_REGION_CACHE_H

#include "memorymap_decl.hpp"
#include "region_decl.hpp"
#include "copydata_decl.hpp"
#include "atomic_decl.hpp"
#include "workdescriptor_fwd.hpp"
#include "processingelement_fwd.hpp"
#include "deviceops_decl.hpp"
#include "regiondirectory_decl.hpp"

namespace nanos {

   class AllocatedChunk {
      private:
         Lock                              _lock;
         uint64_t                          _address;
         uint64_t                          _hostAddress;
         std::size_t                       _size;
         bool                              _dirty;
         std::size_t                       _roBytes;
         std::size_t                       _rwBytes;
         
         RegionTree< CachedRegionStatus > *_regions;

      public:
         //AllocatedChunk( );
         AllocatedChunk( uint64_t addr, uint64_t hostAddr, std::size_t size );
         AllocatedChunk( AllocatedChunk const &chunk );
         AllocatedChunk &operator=( AllocatedChunk const &chunk );
         ~AllocatedChunk();

         uint64_t getAddress() const;
         uint64_t getHostAddress() const;
         std::size_t getSize() const;
         bool isDirty() const;
         void setHostAddress( uint64_t addr );

         void addReadRegion( Region const &reg, unsigned int version, std::set< DeviceOps * > &currentOps, std::list< Region > &notPresentRegions, DeviceOps *ops, bool alsoWriteReg );
         void addWriteRegion( Region const &reg, unsigned int version );
         void clearRegions();
         RegionTree< CachedRegionStatus > *getRegions();
         bool isReady( Region reg );
         void lock();
         void unlock();
   };

   class CacheCopy;
   
   class RegionCache {
      public:
         enum CacheOptions {
            ALLOC_FIT,
            ALLOC_WIDE
         };
      private:
         MemoryMap<AllocatedChunk>  _chunks;
         Lock                       _lock;
         Device                    &_device;
         ProcessingElement         &_pe;
         CacheOptions               _flags;

         typedef MemoryMap<AllocatedChunk>::MemChunkList ChunkList;
         typedef MemoryMap<AllocatedChunk>::ConstMemChunkList ConstChunkList;

         class Op {
               RegionCache &_parent;
            public:
               Op( RegionCache &parent ) : _parent ( parent ) { }
               RegionCache &getParent() const { return _parent; }
               virtual void doNoStrided( int dataLocation, uint64_t devAddr, uint64_t hostAddr, std::size_t size, DeviceOps *ops, WD const &wd ) = 0;
               virtual void doStrided( int dataLocation, uint64_t devAddr, uint64_t hostAddr, std::size_t size, std::size_t count, std::size_t ld, DeviceOps *ops, WD const &wd ) = 0;
         };

         class CopyIn : public Op {
            public:
               CopyIn( RegionCache &parent ) : Op( parent ) {}
               void doNoStrided( int dataLocation, uint64_t devAddr, uint64_t hostAddr, std::size_t size, DeviceOps *ops, WD const &wd ) ;
               void doStrided( int dataLocation, uint64_t devAddr, uint64_t hostAddr, std::size_t size, std::size_t count, std::size_t ld, DeviceOps *ops, WD const &wd ) ;
         } _copyInObj;

         class CopyOut : public Op {
            public:
               CopyOut( RegionCache &parent ) : Op( parent ) {}
               void doNoStrided( int dataLocation, uint64_t devAddr, uint64_t hostAddr, std::size_t size, DeviceOps *ops, WD const &wd ) ;
               void doStrided( int dataLocation, uint64_t devAddr, uint64_t hostAddr, std::size_t size, std::size_t count, std::size_t ld, DeviceOps *ops, WD const &wd ) ;
         } _copyOutObj;

         void doOp( Op *opObj, Region const &hostMem, uint64_t devBaseAddr, unsigned int location, DeviceOps *ops, WD const &wd ); 
         //void _generateRegionOps( Region const &reg, std::map< uintptr_t, MemoryMap< uint64_t > * > &opMap );

      public:
         RegionCache( ProcessingElement &pe, Device &cacheArch, enum CacheOptions flags );
         AllocatedChunk *getAddress( CopyData const &d, RegionTree< CachedRegionStatus > *&regsToInvalidate );
         AllocatedChunk *getAddress( uint64_t hostAddr, std::size_t len );
         void syncRegion( Region const &r ) ;
         void syncRegion( std::list< std::pair< Region, CacheCopy * > > const &regions, WD const &wd ) ;
         unsigned int getMemorySpaceId();
         /* device stubs */
         void _copyIn( uint64_t devAddr, uint64_t hostAddr, std::size_t len, DeviceOps *ops, WD const &wd );
         void _copyOut( uint64_t hostAddr, uint64_t devAddr, std::size_t len, DeviceOps *ops, WD const &wd );
         void _syncAndCopyIn( unsigned int syncFrom, uint64_t devAddr, uint64_t hostAddr, std::size_t len, DeviceOps *ops, WD const &wd );
         void _copyDevToDev( unsigned int copyFrom, uint64_t devAddr, uint64_t hostAddr, std::size_t len, DeviceOps *ops, WD const &wd );
         void _copyInStrided1D( uint64_t devAddr, uint64_t hostAddr, std::size_t len, std::size_t numChunks, std::size_t ld, DeviceOps *ops, WD const &wd );
         void _copyOutStrided1D( uint64_t hostAddr, uint64_t devAddr, std::size_t len, std::size_t numChunks, std::size_t ld, DeviceOps *ops, WD const &wd );
         void _syncAndCopyInStrided1D( unsigned int syncFrom, uint64_t devAddr, uint64_t hostAddr, std::size_t len, std::size_t numChunks, std::size_t ld, DeviceOps *ops, WD const &wd );
         void _copyDevToDevStrided1D( unsigned int copyFrom, uint64_t devAddr, uint64_t hostAddr, std::size_t len, std::size_t numChunks, std::size_t ld, DeviceOps *ops, WD const &wd );
         /* *********** */
         void copyIn( Region const &hostMem, uint64_t devBaseAddr, unsigned int location, DeviceOps *ops, WD const &wd ); 
         void copyOut( Region const &hostMem, uint64_t devBaseAddr, DeviceOps *ops, WD const &wd ); 
         void lock();
         void unlock();
         bool tryLock();
         bool canCopyFrom( RegionCache const &from ) const;
         Device const &getDevice() const;
         unsigned int getNodeNumber() const;
         ProcessingElement &getPE() const;
   };

   class CacheCopy {

      private:
         CopyData const &_copy;
         AllocatedChunk *_cacheEntry;
         std::list< std::pair<Region, CachedRegionStatus const &> > _cacheDataStatus;
         Region _region;
         uint64_t _offset;
         unsigned int _version;
         NewRegionDirectory::LocationInfoList _locations;
         DeviceOps _operations;
         std::set< DeviceOps * > _otherPendingOps;

      public:
         CacheCopy();
         CacheCopy( WD const &wd, unsigned int index );
         
         bool isReady();
         void setUpDeviceAddress( RegionCache *targetCache, NewRegionDirectory *dir );
         void generateCopyInOps( RegionCache *targetCache, std::map<unsigned int, std::list< std::pair< Region, CacheCopy * > > > &opsBySourceRegions ) ;

         NewRegionDirectory::LocationInfoList const &getLocations() const;
         uint64_t getDeviceAddress() const;
         DeviceOps *getOperations();
         Region const &getRegion() const;
         unsigned int getVersion() const;
         CopyData const & getCopyData() const;
   };


   class CacheController {

      private:
         WD const &_wd;
         unsigned int _numCopies;
         CacheCopy *_cacheCopies;
         RegionCache *_targetCache;  

      public:
         CacheController();
         CacheController( WD const &wd );
         ~CacheController();
         bool isCreated() const;
         void preInit( );
         void copyDataIn( RegionCache *targetCache );
         bool dataIsReady() const;
         uint64_t getAddress( unsigned int copyIndex ) const;
         void copyDataOut();

         CacheCopy *getCacheCopies() const;
         RegionCache *getTargetCache() const;
   };
}

#endif
