/*
 * Copyright (c) 2014, Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of Sandia Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdio.h>                      // for NULL
#include "refine_map.h"                 // for refine_edata
#include "structs.h"

void 
update_cube_edata (
    int vertex,		/* graph vertex being worked on */
    int dim,			/* mesh dimension to be adjusted */
    struct refine_edata *edata,	/* data structure for edge preferences */
    struct refine_vdata *vdata,	/* data structure for vertex preferences */
    struct vtx_data **comm_graph,	/* communication graph */
    int *node2vtx,		/* maps processors to comm_graph vtxs */
    int *vtx2node,		/* maps comm_graph vtxs to processors */
    int nsets_tot,		/* total number of processors */
    double *best_desire,		/* best desire seen */
    int imax,			/* offset in desire_ptr array */
    struct refine_edata **desire_ptr	/* buckets for desire values */
)
{
    struct refine_edata *eguy;	/* data for desired edge */
    float     old_desire;	/* original desire for edge to flip */
    float     new_desire;	/* new desire for edge to flip */
    int       node;		/* node number of vertex */
    int       k;		/* index into desire_ptr array */
    double    compute_cube_edata();
    struct refine_edata *find_edge_cube();

    node = vtx2node[vertex];
    eguy = find_edge_cube(node, dim, edata, nsets_tot);

    old_desire = eguy->swap_desire;
    new_desire = compute_cube_edata(eguy, vdata, nsets_tot, comm_graph, node2vtx);

    if (new_desire != old_desire) {	/* Update linked list if necessary. */
	eguy->swap_desire = new_desire;
	if (new_desire > *best_desire) *best_desire = new_desire;

	/* Remove eguy from it's current place in list. */
	if (eguy->prev == NULL) {
	    /* Round up for index into desire_ptr. */
	    if (old_desire >= 0) {
	        k = old_desire;
	        if (k != old_desire) k++;
	    }
	    else {
	        k = -old_desire;
	        if (k != -old_desire) k++;
	        k = -k;
	    }
	    k += imax;
	    desire_ptr[k] = eguy->next;
	}
	else {
	    eguy->prev->next = eguy->next;
	}
	if (eguy->next != NULL)
	    eguy->next->prev = eguy->prev;

	/* Now add eguy to it's new desire bucket. */
	if (new_desire >= 0) {
	    k = new_desire;
	    if (k != new_desire) k++;
	}
	else {
	    k = -new_desire;
	    if (k != -new_desire) k++;
	    k = -k;
	}
	k += imax;

	eguy->prev = NULL;
	eguy->next = desire_ptr[k];
	if (desire_ptr[k] != NULL) desire_ptr[k]->prev = eguy;
	desire_ptr[k] = eguy;
    }
}
