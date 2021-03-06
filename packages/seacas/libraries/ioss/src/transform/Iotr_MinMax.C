// Copyright(C) 1999-2010
// Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software.
//         
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <transform/Iotr_MinMax.h>
#include <Ioss_Field.h>                 // for Field, etc
#include <Ioss_VariableType.h>          // for VariableType
#include <cmath>                        // for fabs
#include <cstddef>                     // for size_t
#include <cstdlib>                     // for abs
#include <algorithm>                    // for max_element, min_element
#include <string>                       // for operator==, string
#include "Ioss_Transform.h"             // for Factory, Transform


namespace {
  bool IntAbsLess(int elem1, int elem2)
  {return std::abs(elem1) < std::abs(elem2);}
  bool doubleAbsLess(double elem1, double elem2)
  {return std::fabs(elem1) < std::fabs(elem2);}
}

namespace Iotr {

  const MinMax_Factory* MinMax_Factory::factory()
  {
    static MinMax_Factory registerThis;
    return &registerThis;
  }

  MinMax_Factory::MinMax_Factory()
    : Factory("generic_minmax")
  {
    Factory::alias("generic_minmax", "minimum");
    Factory::alias("generic_minmax", "maximum");
    Factory::alias("generic_minmax", "absolute_minimum");
    Factory::alias("generic_minmax", "absolute_maximum");
  }

  Ioss::Transform* MinMax_Factory::make(const std::string& type) const
  { return new MinMax(type); }

  MinMax::MinMax(const std::string &type)
  {
    if (type == "minimum") {
      doMin = true;
      doAbs = false;
    } else if (type == "maximum") {
      doMin = false;
      doAbs = false;
    } else if (type == "absolute_minimum") {
      doMin = true;
      doAbs = true;
    } else if (type == "absolute_maximum") {
      doMin = false;
      doAbs = true;
    } else {
      doMin = false;
      doAbs = false;
    }
  }

  const Ioss::VariableType
  *MinMax::output_storage(const Ioss::VariableType *in) const
  {
    // Only operates on scalars...
    static const Ioss::VariableType *sca = Ioss::VariableType::factory("scalar");
    if (in == sca) {
      return sca;
    } 
      return nullptr;
    
  }

  int MinMax::output_count(int /* in */) const
  {
    // Returns a single value...
    return 1;
  }

  bool MinMax::internal_execute(const Ioss::Field &field, void *data)
  {
    size_t count = field.transformed_count();
    size_t components = field.transformed_storage()->component_count();
    size_t n = count * components;
    if (field.get_type() == Ioss::Field::REAL) {
      double *rdata = static_cast<double*>(data);
      double value;
      if (doMin) {
	if (doAbs) {
	  value = *std::min_element(&rdata[0], &rdata[n], doubleAbsLess);
	} else {
	  value = *std::min_element(&rdata[0], &rdata[n]);
	}
      } else { // doMax
	if (doAbs) {
	  value = *std::max_element(&rdata[0], &rdata[n], doubleAbsLess);
	} else {
	  value = *std::max_element(&rdata[0], &rdata[n]);
	}
      }
      rdata[0] = value;
    } else if (field.get_type() == Ioss::Field::INTEGER) {
      int *idata = static_cast<int*>(data);
      int value;
      if (doMin) {
	if (doAbs) {
	  value = *std::min_element(&idata[0], &idata[n], IntAbsLess);
	} else {
	  value = *std::min_element(&idata[0], &idata[n]);
	}
      } else { // doMax
	if (doAbs) {
	  value = *std::max_element(&idata[0], &idata[n], IntAbsLess);
	} else {
	  value = *std::max_element(&idata[0], &idata[n]);
	}
      }
      idata[0] = value;
    }
    return true;
  }
}
