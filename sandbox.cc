#include "mem/cache/prefetch/sandbox.hh"

SandboxPrefetcher::SandboxPrefetcher(const SandboxPrefetcherParams *p)
    : QueuedPrefetcher(p), 
    degree(p->degree),
    distance(p->distance)
{
    int i;
    for(i = 0;i< 8; i++){
        Candidates[i] = new SandboxCandidate;
        Candidates[i].offset = i+1;
        Candidates[i].accuracy = 0;
        Candidates[i+8] = new SandboxCandidate;
        Candidates[i+8].offset = (-1)*(i+1);
        Candidates[i+8].accuracy = 0;
    }
}

void
SandboxPrefetcher::calculatePrefetch(const PacketPtr &pkt,
        std::vector<Addr> &addresses)
{
    Addr blkAddr = pkt->getAddr() & ~(Addr)(blkSize-1);
    
}

SandboxPrefetcher*
SandboxPrefetcherParams::create()
{
   return new SandboxPrefetcher(this);
}