C Copyright(C) 2009 Sandia Corporation. Under the terms of Contract
C DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
C certain rights in this software.
C         
C Redistribution and use in source and binary forms, with or without
C modification, are permitted provided that the following conditions are
C met:
C 
C     * Redistributions of source code must retain the above copyright
C       notice, this list of conditions and the following disclaimer.
C 
C     * Redistributions in binary form must reproduce the above
C       copyright notice, this list of conditions and the following
C       disclaimer in the documentation and/or other materials provided
C       with the distribution.
C     * Neither the name of Sandia Corporation nor the names of its
C       contributors may be used to endorse or promote products derived
C       from this software without specific prior written permission.
C 
C THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
C "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
C LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
C A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
C OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
C SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
C LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
C DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
C THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
C (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
C OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

C=======================================================================
      SUBROUTINE TNODES (IFACE, LINKE, NODEF)
C=======================================================================

C   --*** TNODES *** (MESH) Get nodes for tet element's face
C   --
C   --TNODES returns the nodes for a given face of a tetrahedral element.
C   --The 3-tuple sequence defining the face is counter-clockwise looking
C   --into the element.
C   --
C   --Parameters:
C   --   IFACE - IN - the face number
C   --   LINKE - IN - the tetrahedral element connectivity
C   --   NODEF - OUT - the nodes in the extracted face

      INTEGER LINKE(6), NODEF(4)

      INTEGER KFACE(3,4)
      SAVE KFACE
C      --KFACE(*,i) - the indices of the 3 nodes in face i

      DATA KFACE /
     $     1, 2, 4,
     $     2, 3, 4,
     &     1, 4, 3,
     $     1, 3, 2 /

      DO 100 K = 1, 3
         NODEF(K) = LINKE(KFACE(K,IFACE))
  100 CONTINUE
      NODEF(4) = LINKE(KFACE(1,IFACE))
      RETURN
      END
