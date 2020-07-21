// tsig.c ... functions on Tuple Signatures (tsig's)
// part of SIMC signature files
// Written by John Shepherd, March 2020

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"

// make a tuple signature

static Bits compute_codeword(char *attr, int m, int k) {
    Bits cw = newBits(m);
    srandom(hash_any(attr, strlen(attr)));
    for (Count n = 0; n < k; ) {
        int i = random() % m;
        if (!bitIsSet(cw, i)) {
            setBit(cw, i);
            n++;
        }
    }
    return cw;
}

Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	
    Bits sig = newBits(tsigBits(r));
    char **attrs = tupleVals(r, t);
    for (Count i = 0; i < nAttrs(r); i++) {
        if (strncmp(attrs[i], "?", 1) == 0) {
            continue;
        }
        Bits cw = compute_codeword(attrs[i], tsigBits(r), codeBits(r));
        orBits(sig, cw);
        freeBits(cw);
    }
    return sig;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);

	Reln r = q->rel;

    Bits qsig = makeTupleSig(r, q->qstring);
    unsetAllBits(q->pages);

    q->nsigs = 0;
    q->nsigpages = 0;
    for (PageID pid = 0; pid < nTsigPages(r); pid++) {
        Page p = getPage(tsigFile(r), pid);
        for (Offset tid = 0; tid < pageNitems(p); tid++) {
            Bits tsig = newBits(tsigBits(r));
            getBits(p, tid, tsig);
            if (isSubset(qsig, tsig)) {
                setBit(q->pages, (pid * maxTsigsPP(r) + tid) / maxTupsPP(r));
            }
            freeBits(tsig);
        }
        q->nsigs += pageNitems(p);
    }
    q->nsigpages += nTsigPages(r);

}
