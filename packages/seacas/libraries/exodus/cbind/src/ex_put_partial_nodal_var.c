/*
 * Copyright (c) 2005 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
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
/*****************************************************************************
*
* expnv - ex_put_partial_nodal_var
*
* entry conditions - 
*   input parameters:
*       int     exoid                   exodus file id
*       int     time_step               whole time step number
*       int     nodel_var_index         index of desired nodal variable
*       int     start_node              index (1-based) of first node to put
*       int     num_nodes               number of nodal points
*       float*  nodal_var_vals          array of nodal variable values
*
* exit conditions - 
*
* revision history - 
*
*
*****************************************************************************/

#include <stddef.h>                     // for size_t
#include <stdio.h>                      // for sprintf
#include <sys/types.h>                  // for int64_t
#include "exodusII.h"                   // for exerrval, ex_err, etc
#include "exodusII_int.h"               // for EX_WARN, ex_comp_ws, etc
#include "netcdf.h"                     // for nc_inq_varid, NC_NOERR, etc

/*!
 * writes the values of a single nodal variable for a partial block at
 * one single time step to the database; assume the first time step
 * and nodal variable index is 1 
 * \param exoid            exodus file id 
 * \param time_step        whole time step number 
 * \param nodal_var_index  index of desired nodal variable 
 * \param start_node       index of first node to write data for (1-based)
 * \param num_nodes        number of nodal points
 * \param nodal_var_vals   array of nodal variable values
 */

int ex_put_partial_nodal_var (int   exoid,
			int   time_step,
			int   nodal_var_index,
			int64_t   start_node,
			int64_t   num_nodes, 
			const void *nodal_var_vals)

{
  int status;
  int varid;
  size_t start[3], count[3];
  char errmsg[MAX_ERR_LENGTH];

  exerrval = 0; /* clear error code */

  if (ex_large_model(exoid) == 0) {
    /* write values of the nodal variable */
    if ((status = nc_inq_varid(exoid, VAR_NOD_VAR, &varid)) != NC_NOERR) {
      exerrval = status;
      sprintf(errmsg,
              "Warning: could not find nodal variables in file id %d",
              exoid);
      ex_err("ex_put_partial_nodal_var",errmsg,exerrval);
      return (EX_WARN);
    }
    start[0] = --time_step;
    start[1] = --nodal_var_index;
    start[2] = --start_node;

    count[0] = 1;
    count[1] = 1;
    count[2] = num_nodes;
  } else {
    /* nodal variables stored separately, find variable for this variable
       index */
    if ((status = nc_inq_varid(exoid, VAR_NOD_VAR_NEW(nodal_var_index), &varid)) != NC_NOERR) {
      exerrval = status;
      sprintf(errmsg,
              "Warning: could not find nodal variable %d in file id %d",
              nodal_var_index, exoid);
      ex_err("ex_put_partial_nodal_var",errmsg,exerrval);
      return (EX_WARN);
    }

    start[0] = --time_step;
    start[1] = --start_node;

    count[0] = 1;
    count[1] = num_nodes;
    if (num_nodes == 0) {
      start[1] = 0;
}
  }

  if (ex_comp_ws(exoid) == 4) {
    status = nc_put_vara_float(exoid, varid, start, count, nodal_var_vals);
  } else {
    status = nc_put_vara_double(exoid, varid, start, count, nodal_var_vals);
  }

  if (status != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
	    "Error: failed to store nodal variables in file id %d",
	    exoid);
    ex_err("ex_put_partial_nodal_var",errmsg,exerrval);
    return (EX_FATAL);
  }
  return (EX_NOERR);
}
