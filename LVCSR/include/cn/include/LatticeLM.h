/*
 * LatticeLM.h --
 *	Language model using lattice transition probabilities
 *
 * Copyright (c) 2003 SRI International.  All Rights Reserved.
 *
 * @(#)$Header: /home/srilm/devel/lattice/src/RCS/LatticeLM.h,v 1.2 2003/07/13 19:35:54 stolcke Exp $
 *
 */

#ifndef _LatticeLM_h_
#define _LatticeLM_h_

#include <stdio.h>

#include "LM.h"
#include "Lattice.h"
#include "Trellis.h"
#include "Array.h"

class LatticeLM: public LM
{
public:
    LatticeLM(Lattice &lat);

    /*
     * LM interface
     */
    LogP wordProb(VocabIndex word, const VocabIndex *context);
    LogP wordProbRecompute(VocabIndex word, const VocabIndex *context);

    LogP sentenceProb(const VocabIndex *sentence, TextStats &stats);

    Boolean read(File &file, Boolean limitVocab = false);
    void write(File &file);

protected:
    Lattice &lat;			/* our lattice */
    Trellis<NodeIndex> trellis;		/* for DP over lattice nodes */
    const VocabIndex *prevContext;	/* context from last DP */
    unsigned prevPos;			/* position from last DP */
    LogP prefixProb(VocabIndex word, const VocabIndex *context,
				LogP &contextProb, TextStats &stats);
					/* prefix probability */
    Array<VocabIndex> savedContext;	/* saved, rev'd copy of last context */
    unsigned savedLength;		/* length of saved context above */
};

#endif /* _LatticeLM_h_ */

