#include "mem/cache/prefetch/SPP.hh"
#include <list>
#include "sim/system.hh"

SPPPrefetcher::SPPPrefetcher(const SPPPrefetcherParams *p)
    : QueuedPrefetcher(p),
    pageBytes(system->getPageBytes())
{
    thresold = 0.2;
    prefetch_control = 0.9;
    int i;

/*Initialise Signature Table*/
    for (i = 0;i< 256;i++){
        ST[i]->last_offset = 0;
        ST[i]->signature.val = 0;
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
        GHR[i]->signature.val = 0;
        GHR[i]->pathProb = 0;
        GHR[i]->Delta = 0;
    }
}

void
SPPPrefetcher::calculatePrefetch(const PacketPtr &pkt,
        std::vector<Addr> &addresses)
{
    Addr blkAddr = pkt->getAddr() & ~(Addr)(blkSize-1);
    int totalblk = pageBytes/blkSize;
    int pageno = roundDown(blkAddr, pageBytes) // To get physical page number of current access.
    // int pageno = blkAddr/pageSize;  // To get physical page number of current access.
    int offset = (blkAddr%pageBytes)/blkSize;  // To get current offset from start of page for current address.

    
    int del = offset - ST[pageno]->last_offset;    
    Sig sig; 
    sig.val= ST[pageno]->signature.val;
     float pd ;
/*Check Global History Table, New page getting allocated*/   
    if (sig.val == 0){
        for (int i = 0; i < 256; ++i)
        {
            if((((totalblk -GHR[i]->last_offset) + GHR[i]->Delta)%totalblk) == offset){
                pd = GHR[i]->pathProb;
                sig.val = GHR[i]->signature.val;
                (ST[pageno]->signature).val = sig.val;
            }
        }
        
    }
    else{
        pd = 1.0;
    }
    //Prefetching
   
    Sig tempsig;
    tempsig.val = sig.val;
    Addr currAddr = blkAddr;
    while(pd >= thresold){
        float maxpd = 0.0;
        int index = -1;
        for(int i = 0;i<4;i++){             // Find out optimum offset
            if (PT[tempsig.val]->Delta[i] != 0){
                float temppd = (float)(PT[tempsig.val]->cDelta[i])/PT[tempsig.val]->Csig;
                if (temppd > maxpd){
                    maxpd = temppd;
                    index = i;
                }
            }
        }
        if (index != -1){
            pd = prefetch_control * maxpd * pd; // Calculate prob of this prefetch
            if (pd >= thresold){
                int off = PT[tempsig.val]->Delta[index];
                Addr pf_addr = currAddr + blkSize * off;

/* Check if offset crosses page boundaries, If crossing reset entries of current page and consult GHR*/
                if (samePage(pf_addr, currAddr)) {
                    addresses.push_back(pf_addr);
                    DPRINTF(HWPrefetch, "Queuing prefetch to %#x.\n", pf_addr);
                    tempsig.val = (tempsig.val << 3) ^ delta;
                    currAddr = pf_addr;
                }
                else{
                    int currBlk = (currAddr%pageBytes)/blkSize;
                    flaot tpd = 1.0;
                    int select = -1;
                    for (int i = 0;i<256;i++){
                        if (GHR[i]->pathProb < tpd){
                            tpd = GHR[i]->pathProb;
                            select = i;
                        }
                    }
                    GHR[select]->pathProb = pd;
                    GHR[select]->signature.val = tempsig.val;
                    GHR[select]->last_offset = currBlk;
                    GHR[select]->Delta = off;
                }               
                
            }
        }
        else{
            pd = 0;
        }
    }

// Update Signature Table and Pattern Table
    ST[pageno]-> last_offset = offset;
    (ST[pageno]->signature).val = ((ST[pageno]->signature).val << 3) ^ delta;
    PT[sig.val]->Csig++; //Have to implement saturating counters.
    bool flag = false;
    int index = 0;


    while(index < 4){ 
        if (PT[sig.val]->Delta[index] == del){
            flag = true;
            break;
        }
    }

// Delta found to be incremented
    if(flag == true){
        PT[sig.val]->cDelta[index]++; //Have to implement saturating counters.
    }
    else{
        bool flag = false;
        int index = 0;
// There exist an empty spot . 4 deltas have not been allocated.
        while(index < 4){
            if (PT[sig.val]->Delta[index] == 0){
                flag = true;
                break;
            }
        }
        if (flag){
            PT[sig.val]->Delta[index] = del;
            PT[sig.val]->cDelta[index] = 1;
        }
// We have to fill new delta and hence have to replace an old with lowest probability
        else{

        }
    }
}

SPPPrefetcher*
SPPPrefetcherParams::create()
{
   return new SPPPrefetcher(this);
}

bool
SPPPrefetcher::samePage(Addr a, Addr b) const
{
    return roundDown(a, pageBytes) == roundDown(b, pageBytes);
}