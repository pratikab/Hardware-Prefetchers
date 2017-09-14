#include "mem/cache/prefetch/sandbox.hh"
#include "debug/HWPrefetch.hh"

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
    valid = false;
    realcand = 0;
}

void
SandboxPrefetcher::calculatePrefetch(const PacketPtr &pkt,
        std::vector<Addr> &addresses)
{
    Addr blkAddr = pkt->getAddr() & ~(Addr)(blkSize-1);
    if (CountMisses == 256)  // Reset function
    {
        CountMisses = 0;
        CurrIndex = 0;
        int i;
        for(i = 0;i < 256 ; i++){
            sandbox[i] = 0;
        }
        int max_acc = -1;
        for (i = 0;i<16;i++){
            if((Candidates[i]->accuracy) > max_acc){
                valid = true;
                realcand = Candidates[i]->offset;
                max_acc = (Candidates[i]->accuracy);
            }
            (Candidates[i]->accuracy) = 0;
        }
        
    }
    int index = CountMisses/16; // Current Active Prefetcher
    int currAddr = blkAddr + blkSize*(Candidates[index]->offset);
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
    if (valid){  // Real Prefetching
        for (uint8_t d = 1; d <= degree; d++) {
            Addr pf_addr = blkAddr + blkSize * (d + realcand);
            if (samePage(pf_addr, blkAddr)) {
                    addresses.push_back(pf_addr);
                    DPRINTF(HWPrefetch, "Queuing prefetch to %#x.\n", pf_addr);
            }
            
        }
    }
}

SandboxPrefetcher* SandboxPrefetcherParams::create()
{
   return new SandboxPrefetcher(this);
}