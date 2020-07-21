// bsig.c ... functions on Tuple Signatures (bsig's)
// part of SIMC signature files
// Written by John Shepherd, March 2020

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"

void findPagesUsingBitSlices(Query q)
{
	assert(q != NULL);

	Reln r = q->rel;
    Bits qsig = makePageSig(r, q->qstring);

    PageID pid = 0;
    Page p = NULL;

    q->pages = newBits(bsigBits(r));
    setAllBits(q->pages);

    q->nsigs = 0;
    q->nsigpages = 0;
    for (Count i = 0; i < psigBits(r); i++) {
        if (!bitIsSet(qsig, i)) {
            continue;
        }
        PageID _pid = i / maxBsigsPP(r);
        if (!(p && _pid == pid)){
            pid  = _pid;
            p = getPage(bsigFile(r), pid);
            q->nsigpages++;
        }

        Bits bsig = newBits(bsigBits(r));
        getBits(p, i % maxBsigsPP(r), bsig);
        for (Offset pos = 0; pos < bsigBits(r); pos++) {
            if (!bitIsSet(bsig, pos)) {
                unsetBit(q->pages, pos);
            }
        }
        q->nsigs++;
        freeBits(bsig);
    }
    freeBits(qsig);
}

