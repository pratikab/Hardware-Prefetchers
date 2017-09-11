#include "mem/cache/prefetch/SPP.hh"

SPPPrefetcher::SPPPrefetcher(const SPPPrefetcherParams *p)
    : QueuedPrefetcher(p)
{
    thresold = 0.2;
    prefetch_control = 0.9;
    int i;

/*Initialise Signature Table*/
    for (i = 0;i< 256;i++){
        ST[i]->last_offset = 0;
        ST[i]->signature = 0;
    }
/*Initialise Pattern Table*/
    for (i = 0;i<4096;i++){
        int j;
        for(j = 0;j<4 ;j++){
            PT[i]->Delta[j] = 0;
            PT[i]->cDelta[j] = 0;
            PT[i]->Csig = 0;
        }
    }
/*Initialise global history table*/
    for (i = 0;i< 256;i++){
        GHR[i]->last_offset = 0;
        GHR[i]->signature = 0;
        GHR[i]->pathProb = 0;
        GHR[i]->Delta = 0;
    }
}

void
SPPPrefetcher::calculatePrefetch(const PacketPtr &pkt,
        std::vector<Addr> &addresses)
{
    Addr blkAddr = pkt->getAddr() & ~(Addr)(blkSize-1);
    int pageno = blkAddr/pageSize;  // To get physical page number of current access.
    int offset = blkAddr%pageSize;  // To get current offset from start of page for current address.

    
    int del = offset - ST[pageno]->last_offset;    
    unsigned sig: 12 = ST[pageno]->signature;
    if (sig == 0){
        /*Check Global History Table, New page getting allocated*/
    }

    //Prefetching
    float pd = 1.0;
    unsigned tempsig: 12 = sig;
    while(pd >= thresold){
        float maxpd = 0.0;
        int index = -1;
        for(int i = 0;i<4;i++){             // Find out optimum offset
            if (PT[tempsig]->Delta[i] != 0){
                float temppd = (float)(PT[tempsig]->cDelta[i])/PT[sig]->Csig;
                if (temppd > maxpd){
                    maxpd = temppd;
                    index = i;
                }
            }
        }
        if (index != i){
            pd = prefetch_control * maxpd * pd; // Calculate prob of this prefetch
            if (pd >= thresold){
                int off = PT[tempsig]->Delta[index];

/* Check if offset crosses page boundaries, If crossing reset entries of current page and consult GHR*/

                Addr pf_addr = blkAddr + blkSize * off;
                addresses.push_back(pf_addr);
                DPRINTF(HWPrefetch, "Queuing prefetch to %#x.\n", pf_addr);
                tempsig = (tempsig << 3) ^ delta;
            }
        }
        else{
            pd = 0;
        }
    }

    // Update Signature Table and Pattern Table
    ST[pageno]-> last_offset = offset;
    ST[pageno]->signature = (ST[pageno]->signature << 3) ^ delta;
    PT[sig]->Csig++; //Have to implement saturating counters.
    bool flag = false;
    int index = 0;
    while(index < 4){
        if (PT[sig]->Delta[index] == del){
            flag = true;
            break;
        }
    }
    if(flag == true){
        PT[sig]->cDelta[index]++; //Have to implement saturating counters.
    }
    else{
        bool flag = false;
        int index = 0;
        while(index < 4){
            if (PT[sig]->Delta[index] == 0){
                flag = true;
                break;
            }
        }
        PT[sig]->Delta[index] = del;
        PT[sig]->cDelta[index] = 1;
    }
}

SPPPrefetcher*
SPPPrefetcherParams::create()
{
   return new SPPPrefetcher(this);
}