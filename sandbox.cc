#include "mem/cache/prefetch/sandbox.hh"

SandboxPrefetcher::SandboxPrefetcher(const SandboxPrefetcherParams *p)
    : QueuedPrefetcher(p), 
    degree(p->degree),
    distance(p->distance)
{
    int i;
    CountMisses = 0;
    for(i = 0;i < 256 ; i++){
        sandbox[i] = 0;
    }
    CurrIndex = 0;
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
    int currAddr = blkaddr + blkSize*(Candidates[index]->offset);
    sandbox[CurrIndex] = currAddr;
    CountMisses++;
    CurrIndex++;
    int i ;
    for(i = 0;i<CurrIndex;i++){
        if (sandbox[i] == blkAddr){
            (Candidates[index]->accuracy)++;
            break;
        }
    }
}

SandboxPrefetcher*
SandboxPrefetcherParams::create()
{
   return new SandboxPrefetcher(this);
}