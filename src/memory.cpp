/*! \file memory.cpp

    Class for accessing memory usage
*/

#include "memory.h"
#include "string_functions.h"

#include <string>

using namespace std;
using namespace   chrono;

// -----------  memory_information ----------------

/*! \class  memory_information
    \brief  Obtain and make available memory information
*/

/*! \brief          Possibly read /proc/meminfo
    \param  force   whether to force reading of /proc/meminfo regardless of <i>_last_update_time</i> and <i>_minimum_interval</i>
*/
void memory_information::_get_meminfo(const bool force)
{ constexpr uint64_t BYTES_PER_KB { 1024 };             // https://unix.stackexchange.com/questions/263881/convert-meminfo-kb-to-bytes

  const system_clock::time_point now { system_clock::now() };

  if ( force or ( (now - _last_update_time) >  _minimum_interval) )      // update only if forced or if enough time has passed
  { const vector<string> file_lines { to_lines(squash(read_file("/proc/meminfo"s))) };

    _last_update_time = now;                                // update the time of last update

    lock_guard<mutex> memory_lock(_memory_mutex);

    for (const auto& line : file_lines)
    { const vector<string> fields { split_string(line, SPACE_STR) };

      if ( (fields.size() < 2) or (fields.size() > 3) )
      { cerr << "Fatal error in memory_information::_get_meminfo(void)" << endl;
        exit(-1);
      }

      const string   name  { substring(fields[0], 0, fields[0].length() - 1) };    // remove the terminating colon
      const string   unit  { (fields.size() == 3) ? fields[2] : string() };

      if (!unit.empty() and unit != "kB"s)
      { cerr << "Fatal unit error in memory_information::_get_meminfo(void)" << endl;
        exit(-1);
      }

      uint64_t value { from_string<uint64_t>(fields[1]) };

      if (!unit.empty())
        value *= BYTES_PER_KB;

      _values[name] = value;        // creates if necessary
    }
  }
}

/*! \brief              Constructor
    \param  min_int     the minimum interval between reads of /proc/meminfo
*/
memory_information::memory_information(const std::chrono::system_clock::duration min_int) :
  _minimum_interval(min_int),
  _last_update_time { std::chrono::system_clock::now() - 2 * _minimum_interval }        // force update when _get_meminfo() is called
{ _get_meminfo(); }

/*! \brief          Convert to printable string
    \param  force   whether to force reading of /proc/meminfo regardless of <i>_last_update_time</i> and <i>_minimum_interval</i>
    \return         the state of the object as a printable string
*/
const string memory_information::to_string(const bool force)
{ _get_meminfo(force);

  const string rv =   "Total             = "s + comma_separated_string(mem_total()) + EOL
                    + "Free              = "s + comma_separated_string(mem_free()) + EOL
                    + "Available         = "s + comma_separated_string(mem_available()) + EOL
                    + "Buffers           = "s + comma_separated_string(buffers()) + EOL
                    + "Cached            = "s + comma_separated_string(cached()) + EOL
                    + "SwapCached        = "s + comma_separated_string(swap_cached()) + EOL
                    + "Active            = "s + comma_separated_string(active()) + EOL
                    + "Inactive          = "s + comma_separated_string(inactive()) + EOL
                    + "Active(anon)      = "s + comma_separated_string(active_anon()) + EOL
                    + "Inactive(anon)    = "s + comma_separated_string(inactive_anon()) + EOL
                    + "Active(file)      = "s + comma_separated_string(active_file()) + EOL
                    + "Inactive(file)    = "s + comma_separated_string(inactive_file()) + EOL
                    + "Unevictable       = "s + comma_separated_string(unevictable()) + EOL
                    + "Mlocked           = "s + comma_separated_string(mlocked()) + EOL
                    + "SwapTotal         = "s + comma_separated_string(swap_total()) + EOL
                    + "SwapFree          = "s + comma_separated_string(swap_free()) + EOL
                    + "Dirty             = "s + comma_separated_string(dirty()) + EOL
                    + "Writeback         = "s + comma_separated_string(writeback()) + EOL
                    + "AnonPages         = "s + comma_separated_string(anon_pages()) + EOL
                    + "Mapped            = "s + comma_separated_string(mapped()) + EOL
                    + "Shmem             = "s + comma_separated_string(shmem()) + EOL
                    + "Slab              = "s + comma_separated_string(slab()) + EOL
                    + "SReclaimable      = "s + comma_separated_string(s_reclaimable()) + EOL
                    + "SUnreclaim        = "s + comma_separated_string(s_unreclaim()) + EOL
                    + "KernelStack       = "s + comma_separated_string(kernel_stack()) + EOL
                    + "PageTables        = "s + comma_separated_string(page_tables()) + EOL
                    + "NFS_Unstable      = "s + comma_separated_string(nfs_unstable()) + EOL
                    + "Bounce            = "s + comma_separated_string(bounce()) + EOL
                    + "WritebackTmp      = "s + comma_separated_string(writeback_tmp()) + EOL
                    + "CommitLimit       = "s + comma_separated_string(commit_limit()) + EOL
                    + "Committed_AS      = "s + comma_separated_string(committed_as()) + EOL
                    + "VmallocTotal      = "s + comma_separated_string(vmalloc_total()) + EOL
                    + "VmallocUsed       = "s + comma_separated_string(vmalloc_used()) + EOL
                    + "VmallocChunk      = "s + comma_separated_string(vmalloc_chunk()) + EOL
                    + "HardwareCorrupted = "s + comma_separated_string(hardware_corrupted()) + EOL
                    + "AnonHugePages     = "s + comma_separated_string(anon_huge_pages()) + EOL
                    + "ShmemHugePages    = "s + comma_separated_string(shmem_huge_pages()) + EOL
                    + "ShmemPmdMapped    = "s + comma_separated_string(shmem_pmd_mapped()) + EOL
                    + "HugePages_Total   = "s + comma_separated_string(huge_pages_total()) + EOL
                    + "HugePages_Free    = "s + comma_separated_string(huge_pages_free()) + EOL
                    + "HugePages_Rsvd    = "s + comma_separated_string(huge_pages_rsvd()) + EOL
                    + "HugePages_Surp    = "s + comma_separated_string(huge_pages_surp()) + EOL
                    + "Hugepagesize      = "s + comma_separated_string(hugepagesize()) + EOL
                    + "DirectMap4k       = "s + comma_separated_string(direct_map_4k()) + EOL
                    + "DirectMap2M       = "s + comma_separated_string(direct_map_2m());

  return rv;
}

