

#ifndef __MEM_CACHE_PREFETCH_SPP_HH__
#define __MEM_CACHE_PREFETCH_SPP_HH__

#include "mem/cache/prefetch/queued.hh"
#include "params/SPP.hh"


class SPPPrefetcher : public QueuedPrefetcher
{
  protected:
    class SignatureTableEntry {
      public:
        int last_offset; // Offset for immediate prefetching
        unsigned signature: 12;
    };
    class PatternTableEntry {
      public:
        int Delta[4];
        int cDelta[4];
        int Csig;
    };
    class GlobalHistoryTableEntry{
      public:
        unsigned signature: 12;
        float pathProb;
        int last_offset;
        int Delta;
    };
    SignatureTableEntry * ST[256];
    GlobalHistoryTableEntry * GHR[256];
    PatternTableEntry * PT[4096];
    float prefetch_control;
    float thresold;
  public:
    SPPPrefetcher(const SPPPrefetcherParams *p);

    ~SPPPrefetcher() {}

    void calculatePrefetch(const PacketPtr &pkt, std::vector<Addr> &addresses);
};

#endif // __MEM_CACHE_PREFETCH_SPP_HH__
