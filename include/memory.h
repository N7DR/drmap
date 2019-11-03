// $Id: memory.h 6 2019-10-04 22:18:44Z n7dr $

/*! \file memory.h

    Class for accessing memory usage
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "macros.h"

#include <chrono>
#include <mutex>
#include <unordered_map>

using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;

// -----------  memory_information ----------------

/*! \class  memory_information
    \brief  Obtain and make available memory information
*/

class memory_information
{
protected:

  std::chrono::system_clock::time_point _last_update_time;         ///< time at which /proc/meminfo was last read
  std::chrono::system_clock::duration   _minimum_interval;         ///< minimum interval between unforced reads of /proc/meminfo

  std::unordered_map<std::string /* name */, uint64_t /* value */> _values; ///< all the values from /proc/meminfo; see "man free", "man procfs"

/*! \brief          Possibly read /proc/meminfo
    \param  force   whether to force reading of /proc/meminfo regardless of <i>_last_update_time</i> and <i>_minimum_interval</i>
*/
  void _get_meminfo(const bool force = false);                      ///< possibly read /proc/meminfo

  std::mutex _memory_mutex;                                         ///< used to make it thread-safe

public:

/*! \brief              Constructor
    \param  min_int     the minimum interval between reads of /proc/meminfo
*/
  memory_information(const std::chrono::system_clock::duration min_int = 1s);

  READ_AND_WRITE(minimum_interval);         ///< minimum interval between unforced reads of /proc/meminfo

/// get MemTotal
  inline const uint64_t mem_total(const bool force = false)
    { return ( _get_meminfo(force), _values.at("MemTotal"s) ); }             // MemTotal:        8178256 kB

/// get MemFree
  inline const uint64_t mem_free(const bool force = false)
    { return ( _get_meminfo(force), _values.at("MemFree"s) ); }              // MemFree:          551600 kB

/// get MemAvailable
  inline const uint64_t mem_available(const bool force = false)
    { return ( _get_meminfo(force), _values.at("MemAvailable"s) ); }         // MemAvailable:    2265744 kB

/// get Buffers
  inline const uint64_t buffers(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Buffers"s) ); }              // Buffers:          271592 kB

/// get Cached
  inline const uint64_t cached(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Cached"s) ); }               // Cached:          1462272 kB

/// get SwapCached
  inline const uint64_t swap_cached(const bool force = false)
    { return ( _get_meminfo(force), _values.at("SwapCached"s) ); }           // SwapCached:       181216 kB

/// get Active
  inline const uint64_t active(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Active"s) ); }               // Active:          4381312 kB

/// get Inactive
  inline const uint64_t inactive(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Inactive"s) ); }             // Inactive:        1240504 kB

/// get Active(anon)
  inline const uint64_t active_anon(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Active(anon)"s) ); }         // Active(anon):    3344136 kB

/// get Inactive(anon)
  inline const uint64_t inactive_anon(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Inactive(anon)"s) ); }       // Inactive(anon):   719528 kB

/// get Active(file)
  inline const uint64_t active_file(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Active(file)"s) ); }         // Active(file):    1037176 kB

/// get Inactive(file)
  inline const uint64_t inactive_file(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Inactive(file)"s) ); }       // Inactive(file):   520976 kB

/// get Unevictable
  inline const uint64_t unevictable(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Unevictable"s) ); }          // Unevictable:         112 kB

/// get Mlocked
  inline const uint64_t mlocked(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Mlocked"s) ); }              // Mlocked:             112 kB

/// get SwapTotal
  inline const uint64_t swap_total(const bool force = false)
    { return ( _get_meminfo(force), _values.at("SwapTotal"s) ); }            // SwapTotal:      15615864 kB

  inline const uint64_t swap_free(const bool force = false)
    { return ( _get_meminfo(force), _values.at("SwapFree"s) ); }             // SwapFree:       14495636 kB

  inline const uint64_t dirty(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Dirty"s) ); }                // Dirty:              1376 kB

  inline const uint64_t writeback(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Writeback"s) ); }            // Writeback:             0 kB

  inline const uint64_t anon_pages(const bool force = false)
    { return ( _get_meminfo(force), _values.at("AnonPages"s) ); }            // AnonPages:       3882372 kB

  inline const uint64_t mapped(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Mapped"s) ); }               // Mapped:           384940 kB

  inline const uint64_t shmem(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Shmem"s) ); }                // Shmem:            175676 kB

  inline const uint64_t slab(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Slab"s) ); }                 // Slab:             655936 kB

  inline const uint64_t s_reclaimable(const bool force = false)
    { return ( _get_meminfo(force), _values.at("SReclaimable"s) ); }         // SReclaimable:     459916 kB

  inline const uint64_t s_unreclaim(const bool force = false)
    { return ( _get_meminfo(force), _values.at("SUnreclaim"s) ); }           // SUnreclaim:       196020 kB

  inline const uint64_t kernel_stack(const bool force = false)
    { return ( _get_meminfo(force), _values.at("KernelStack"s) ); }          // KernelStack:       21568 kB

  inline const uint64_t page_tables(const bool force = false)
    { return ( _get_meminfo(force), _values.at("PageTables"s) ); }           // PageTables:        80948 kB

  inline const uint64_t nfs_unstable(const bool force = false)
    { return ( _get_meminfo(force), _values.at("NFS_Unstable"s) ); }         // NFS_Unstable:          0 kB

  inline const uint64_t bounce(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Bounce"s) ); }               // Bounce:                0 kB

  inline const uint64_t writeback_tmp(const bool force = false)
    { return ( _get_meminfo(force), _values.at("WritebackTmp"s) ); }         // WritebackTmp:          0 kB

  inline const uint64_t commit_limit(const bool force = false)
    { return ( _get_meminfo(force), _values.at("CommitLimit"s) ); }          // CommitLimit:    19704992 kB

  inline const uint64_t committed_as(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Committed_AS"s) ); }         // Committed_AS:   10977852 kB

  inline const uint64_t vmalloc_total(const bool force = false)
    { return ( _get_meminfo(force), _values.at("VmallocTotal"s) ); }         // VmallocTotal:   34359738367 kB

  inline const uint64_t vmalloc_used(const bool force = false)
    { return ( _get_meminfo(force), _values.at("VmallocUsed"s) ); }          // VmallocUsed:           0 kB

  inline const uint64_t vmalloc_chunk(const bool force = false)
    { return ( _get_meminfo(force), _values.at("VmallocChunk"s) ); }         // VmallocChunk:          0 kB

  inline const uint64_t hardware_corrupted(const bool force = false)
    { return ( _get_meminfo(force), _values.at("HardwareCorrupted"s) ); }    // HardwareCorrupted:     0 kB

  inline const uint64_t anon_huge_pages(const bool force = false)
    { return ( _get_meminfo(force), _values.at("AnonHugePages"s) ); }        // AnonHugePages:         0 kB

  inline const uint64_t shmem_huge_pages(const bool force = false)
    { return ( _get_meminfo(force), _values.at("ShmemHugePages"s) ); }       // ShmemHugePages:        0 kB

  inline const uint64_t shmem_pmd_mapped(const bool force = false)
    { return ( _get_meminfo(force), _values.at("ShmemPmdMapped"s) ); }       // ShmemPmdMapped:        0 kB

  inline const uint64_t huge_pages_total(const bool force = false)
    { return ( _get_meminfo(force), _values.at("HugePages_Total"s) ); }      // HugePages_Total:       0

  inline const uint64_t huge_pages_free(const bool force = false)
    { return ( _get_meminfo(force), _values.at("HugePages_Free"s) ); }       // HugePages_Free:        0

  inline const uint64_t huge_pages_rsvd(const bool force = false)
    { return ( _get_meminfo(force), _values.at("HugePages_Rsvd"s) ); }       // HugePages_Rsvd:        0

  inline const uint64_t huge_pages_surp(const bool force = false)
    { return ( _get_meminfo(force), _values.at("HugePages_Surp"s) ); }       // HugePages_Surp:        0

  inline const uint64_t hugepagesize(const bool force = false)
    { return ( _get_meminfo(force), _values.at("Hugepagesize"s) ); }         // Hugepagesize:       2048 kB

  inline const uint64_t direct_map_4k(const bool force = false)
    { return ( _get_meminfo(force), _values.at("DirectMap4k"s) ); }          // DirectMap4k:     6185856 kB

  inline const uint64_t direct_map_2m(const bool force = false)
    { return ( _get_meminfo(force), _values.at("DirectMap2M"s) ); }          // DirectMap2M:     2201600 kB

/*! \brief          Convert to printable string
    \param  force   whether to force reading of /proc/meminfo regardless of <i>_last_update_time</i> and <i>_minimum_interval</i>
    \return         the state of the object as a printable string
*/
  const std::string to_string(const bool force = false);
};

#endif /* MEMORY_H */
