

#ifndef __MEM_CACHE_PREFETCH_SANDBOX_HH__
#define __MEM_CACHE_PREFETCH_SANDBOX_HH__

#include "mem/cache/prefetch/queued.hh"
#include "params/SandboxPrefetcher.hh"


class SandboxPrefetcher : public QueuedPrefetcher
{
  protected:
    uint32_t degree;                  	    // Determines the number of prefetch reuquests to be issued at a time
    uint32_t distance;                      // Determines the prefetch distance
   	uint32_t CountMisses;					// Counter of number of misses
    class SandboxCandidate {
      public:
        int offset; // Offset for immediate prefetching
        int accuracy;
    };
    SandboxCandidate * Candidates[16];
    Addr sandbox[256];
    uint32_t CurrIndex;
  public:
    SandboxPrefetcher(const SandboxPrefetcherParams *p);

    ~SandboxPrefetcher() {}

    void calculatePrefetch(const PacketPtr &pkt, std::vector<Addr> &addresses);
};

#endif // __MEM_CACHE_PREFETCH_SANDBOX_HH__
