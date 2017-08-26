#include "mem/cache/prefetch/sandbox.hh"

SandboxPrefetcher::SandboxPrefetcher(const SandboxPrefetcherParams *p)
    : QueuedPrefetcher(p), 
    degree(p->degree),
    distance(p->distance)
{
    int i;
    lastaddr = 0;
    CountMisses = 0;
    for(i = 0;i< 8; i++){
        Candidates[i] = new SandboxCandidate;
        Candidates[i]->offset = i+1;
        Candidates[i]->accuracy = 0;
        Candidates[i+8] = new SandboxCandidate;
        Candidates[i+8]->offset = (-1)*(i+1);
        Candidates[i+8]->accuracy = 0;
    }
}

void
SandboxPrefetcher::calculatePrefetch(const PacketPtr &pkt,
        std::vector<Addr> &addresses)
{
    Addr blkAddr = pkt->getAddr() & ~(Addr)(blkSize-1);
    if (CountMisses == 256) reset(); // Reset function
    int index = CountMisses/16; // Current Active Prefetcher
    int currAddr = lastaddr + blkSize*(Candidates[index]->offset);
    CountMisses++;
    if (currAddr == blkAddr){ // not actually, we need to get response from bloom filter
        (Candidates[index]->accuracy)++;
    }
    lastaddr = blkAddr;
}

SandboxPrefetcher*
SandboxPrefetcherParams::create()
{
   return new SandboxPrefetcher(this);
}