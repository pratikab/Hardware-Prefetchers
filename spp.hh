#ifndef __MEM_CACHE_PREFETCH_SPP_HH__
#define __MEM_CACHE_PREFETCH_SPP_HH__

#include "mem/cache/prefetch/queued.hh"
#include "params/SPPPrefetcher.hh"


class SPPPrefetcher : public QueuedPrefetcher
{
  protected:
    struct Sig
    {
        unsigned val: 12; // 12 bits
    };
    class SignatureTableEntry {
      public:
        int last_offset; // Offset for immediate prefetching
        Sig signature;
    };
    class PatternTableEntry {
      public:
        int Delta[4];
        int cDelta[4];
        int Csig;
    };
    class GlobalHistoryTableEntry{
      public:
        Sig signature;
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
    Addr pageBytes;
    SPPPrefetcher(const SPPPrefetcherParams *p);

    ~SPPPrefetcher() {}
    bool samePage(Addr a, Addr b) const;
    void calculatePrefetch(const PacketPtr &pkt, std::vector<Addr> &addresses);
};

#endif // __MEM_CACHE_PREFETCH_SPP_HH__
