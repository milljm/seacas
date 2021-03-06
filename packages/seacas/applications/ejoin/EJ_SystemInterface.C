
#include "EJ_SystemInterface.h"
#include <SL_tokenize.h>                // for tokenize
#include <ctype.h>                      // for tolower
#include <limits.h>                     // for INT_MAX
#include <stddef.h>                     // for size_t
#include <algorithm>                    // for sort, find, transform
#include <cstdlib>                      // for nullptr, exit, strtod, strtoul, etc
#include <cstring>                      // for strchr, strlen
#include <iostream>                     // for operator<<, basic_ostream, etc
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, allocator, etc
#include "EJ_Version.h"                 // for qainfo
#include "EJ_vector3d.h"                // for vector3d



namespace {
  int case_strcmp(const std::string &s1, const std::string &s2)
  {
    const char *c1 = s1.c_str();
    const char *c2 = s2.c_str();
    for ( ; ; c1++, c2++) {
      if (std::tolower(*c1) != std::tolower(*c2))
	return (std::tolower(*c1) - std::tolower(*c2));
      if (*c1 == '\0')
	return 0;
    }
  }
  void parse_variable_names(const char *tokens, StringIdVector *variable_list);
  void parse_variable_names(const char *tokens, StringIdVector *variable_list);
  void parse_offset(const char *tokens, vector3d *offset);
  void parse_integer_list(const char *tokens, std::vector<int> *list);
  void parse_part_list(const char *tokens, std::vector<int> *list);
  void parse_omissions(const char *tokens, Omissions *omissions,
		       const std::string &basename, bool require_ids);
}

SystemInterface::SystemInterface()
  : outputName_(),
    debugLevel_(0), 
    stepMin_(1), stepMax_(INT_MAX), stepInterval_(1),
    omitNodesets_(false), omitSidesets_(false),
    matchNodeIds_(false), matchNodeXYZ_(false),
    matchElemIds_(false), 
    disableFieldRecognition_(false), ints64bit_(false),
    tolerance_(0.0)
{
  offset_.x = 0.0;
  offset_.y = 0.0;
  offset_.z = 0.0;
  enroll_options();
}

SystemInterface::~SystemInterface() = default;

void SystemInterface::enroll_options()
{
  options_.usage("[options] list_of_files_to_join");

  options_.enroll("help", GetLongOption::NoValue,
		  "Print this summary and exit", nullptr);

  options_.enroll("version", GetLongOption::NoValue,
		  "Print version and exit", nullptr);

  options_.enroll("output", GetLongOption::MandatoryValue,
		  "Name of output file to create",
		  "ejoin-out.e");
  
  options_.enroll("omit_blocks", GetLongOption::MandatoryValue,
		  "Omit the specified part/block pairs. The specification is\n"
		  "\t\tp#:block_id1:block_id2,p#:block_id1. For example, to\n"
		  "\t\tOmit block id 1,3,4 from part 1; block 2 3 4 from part 2;\n"
		  "\t\tand block 8 from part5, specify\n"
		  "\t\t\t '-omit_blocks p1:1:3:4,p2:2:3:4,p5:8'",
		  nullptr);

  options_.enroll("omit_nodesets", GetLongOption::OptionalValue,
		  "If no value, then don't transfer any nodesets to output file.\n"
		  "\t\tIf just p#,p#,... specified, then omit sets on specified parts\n"
		  "\t\tIf p#:id1:id2,p#:id2,id4... then omit the sets with the specified\n"
		  "\t\tid in the specified parts.",
		  nullptr, "ALL");

  options_.enroll("omit_sidesets", GetLongOption::OptionalValue,
		  "If no value, then don't transfer any sidesets to output file.\n"
		  "\t\tIf just p#,p#,... specified, then omit sets on specified parts\n"
		  "\t\tIf p#:id1:id2,p#:id2,id4... then omit the sets with the specified\n"
		  "\t\tid in the specified parts.",
		  nullptr, "ALL");

  options_.enroll("convert_nodal_to_nodesets", GetLongOption::MandatoryValue,
		  "For each part listed (or ALL),\n"
		  "\t\tcreate a nodeset containing the nodes of that part\n"
		  "\t\tand output the nodal fields as nodeset fields instead of nodal fields.\n"
		  "\t\tFormat is comma-separated list of parts (1-based), or ALL",
		  nullptr);

  options_.enroll("match_node_ids", GetLongOption::NoValue,
		  "Combine nodes if their global ids match.",
		  nullptr);
		  
  options_.enroll("match_node_coordinates", GetLongOption::NoValue,
		  "Combine nodes if they are within tolerance distance of each other.",
		  nullptr);
		  
#if 0
  options_.enroll("match_elem_ids", GetLongOption::NoValue,
		  "Combine elements if their global ids match and they are compatible.\n"
		  "\t\tCompatible = same element type, nodes of the two elements match",
		  nullptr);
		  
  options_.enroll("match_element_coordinates", GetLongOption::NoValue,
		  "Combine elements if their centroids are within tolerance distance\n"
		  "\t\tand they are compatible (same element type, nodes match).",
		  nullptr);
#endif
  
  options_.enroll("tolerance", GetLongOption::MandatoryValue,
                  "Maximum distance between two nodes to be considered colocated.",
                  nullptr);

  options_.enroll("offset", GetLongOption::MandatoryValue,
		  "Comma-separated x,y,z offset for coordinates of second mesh.",
		  nullptr);
  
  options_.enroll("steps", GetLongOption::MandatoryValue,
                  "Specify subset of timesteps to transfer to output file.\n"
                  "\t\tFormat is beg:end:step. 1:10:2 --> 1,3,5,7,9\n"
		  "\t\tTo only transfer last step, use '-steps LAST'",
                  "1:");

  options_.enroll("gvar", GetLongOption::MandatoryValue,
		  "Comma-separated list of global variables to be joined or ALL or NONE.",
		  nullptr);

  options_.enroll("evar", GetLongOption::MandatoryValue,
		  "Comma-separated list of element variables to be joined or ALL or NONE.\n"
		  "\t\tVariables can be limited to certain blocks by appending a\n"
		  "\t\tcolon followed by the block id.  E.g. -evar sigxx:10:20",
		  nullptr);

  options_.enroll("nvar", GetLongOption::MandatoryValue,
		  "Comma-separated list of nodal variables to be joined or ALL or NONE.",
		  nullptr);

  options_.enroll("nsetvar", GetLongOption::MandatoryValue,
		  "Comma-separated list of nodeset variables to be joined or ALL or NONE.\n"
		  "\t\tVariables can be limited to certain sets by appending a\n"
		  "\t\tcolon followed by the nodeset id.  E.g. -nsetvar sigxx:10:20",
		  nullptr);

  options_.enroll("ssetvar", GetLongOption::MandatoryValue,
		  "Comma-separated list of sideset variables to be joined or ALL or NONE.\n"
		  "\t\tVariables can be limited to certain sidesets by appending a\n"
		  "\t\tcolon followed by the sideset id.  E.g. -ssetvar sigxx:10:20",
		  nullptr);

  options_.enroll("info_records", GetLongOption::OptionalValue,
		  "If no value specified or not present,\n"
		  "\t\tthen don't transfer any information records to output file.\n"
		  "\t\tIf 'p#,p#,...' specified, then only transfer information records on specified parts\n"
		  "\t\tIf 'all' specified, then transfer all information records",
		  nullptr, "NONE");

  options_.enroll("64-bit", GetLongOption::NoValue,
                  "True if forcing the use of 64-bit integers for the output file",
                  nullptr);

  options_.enroll("disable_field_recognition", GetLongOption::NoValue,
		  "Do not try to combine scalar fields into higher-order fields such as\n"
		  "\t\tvectors or tensors based on the field suffix",
		  nullptr);
  
  options_.enroll("copyright", GetLongOption::NoValue,
		  "Show copyright and license data.",
		  nullptr);
}

bool SystemInterface::parse_options(int argc, char **argv)
{
#if (__SUNPRO_CC == 0x500)
  using namespace std;
#endif

  int option_index = options_.parse(argc, argv);
  if ( option_index < 1 )
    return false;

  if (options_.retrieve("help")) {
    options_.usage();
    std::cerr << "\n\tCan also set options via EJOIN_OPTIONS environment variable.\n";
    std::cerr << "\n\t->->-> Send email to gdsjaar@sandia.gov for ejoin support.<-<-<-\n";
    exit(EXIT_SUCCESS);
  }

  if (options_.retrieve("version")) {
    // Version is printed up front, just exit...
    exit(0);
  }
  
  if (options_.retrieve("copyright")) {
    std::cerr << "\n"
	      << "Copyright(C) 2010 Sandia Corporation.\n"
	      << "\n"
	      << "Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,\n"
	      << "the U.S. Government retains certain rights in this software.\n"
	      << "        \n"
	      << "Redistribution and use in source and binary forms, with or without\n"
	      << "modification, are permitted provided that the following conditions are\n"
	      << "met:\n"
	      << "\n"
	      << "    * Redistributions of source code must retain the above copyright\n"
	      << "      notice, this list of conditions and the following disclaimer.\n"
	      << "\n"
	      << "    * Redistributions in binary form must reproduce the above\n"
	      << "      copyright notice, this list of conditions and the following\n"
	      << "      disclaimer in the documentation and/or other materials provided\n"
	      << "      with the distribution.\n"
	      << "    * Neither the name of Sandia Corporation nor the names of its\n"
	      << "      contributors may be used to endorse or promote products derived\n"
	      << "      from this software without specific prior written permission.\n"
	      << "\n"
	      << "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
	      << "'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
	      << "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
	      << "A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
	      << "OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
	      << "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n"
	      << "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
	      << "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
	      << "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
	      << "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
	      << "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n";
    exit(EXIT_SUCCESS);
  }
  
  // Parse remaining options as directory paths.
  // Note that inputFiles_.size() is the number of parts which can be used in
  // parsing of other options...
  
  if (option_index < argc) {
    while (option_index < argc) {
      inputFiles_.push_back(argv[option_index++]);
    }
  } else {
    std::cerr << "\nERROR: no files specified\n\n";
    return false;
  }

  size_t part_count = inputFiles_.size();
  blockOmissions_.resize(part_count);
  nsetOmissions_.resize(part_count);
  ssetOmissions_.resize(part_count);
  
  // Get options from environment variable also...
  char *options = getenv("EJOIN_OPTIONS");
  if (options != nullptr) {
    std::cerr << "\nThe following options were specified via the EJOIN_OPTIONS environment variable:\n"
	      << "\t" << options << "\n\n";
    options_.parse(options, options_.basename(*argv));
  }

  {
    const char *temp = options_.retrieve("output");
    outputName_ = temp;
  }

  {
    const char *temp = options_.retrieve("offset");
    parse_offset(temp, &offset_);
  }

  {
    const char *temp = options_.retrieve("tolerance");
    if (temp != nullptr) 
      tolerance_ = strtod(temp, nullptr);
  }

  {
    const char *temp = options_.retrieve("steps");
    if (temp != nullptr) {
      parse_step_option(temp);
    }
  }

  {
    const char *temp = options_.retrieve("convert_nodal_to_nodesets");
    if (temp != nullptr) {
      parse_integer_list(temp, &nodesetConvertParts_);
      std::sort(nodesetConvertParts_.begin(), nodesetConvertParts_.end());
    }
  }

  {
    const char *temp = options_.retrieve("info_records");
    if (temp != nullptr) {
      parse_part_list(temp, &infoRecordParts_);
      std::sort(infoRecordParts_.begin(), infoRecordParts_.end());
    }
  }

  {
    const char *temp = options_.retrieve("omit_blocks");
    parse_omissions(temp, &blockOmissions_, "block", true);
  }

  {
    const char *temp = options_.retrieve("omit_nodesets");
    if (temp != nullptr) {
      if (case_strcmp("ALL", temp) == 0)
	omitNodesets_ = true;
      else
	parse_omissions(temp, &nsetOmissions_, "nodelist", false);
    } else {
      omitNodesets_ = false;
    }
  }
  
  {
    const char *temp = options_.retrieve("omit_sidesets");
    if (temp != nullptr) {
      if (case_strcmp("ALL", temp) == 0)
	omitSidesets_ = true;
      else
	parse_omissions(temp, &ssetOmissions_, "surface", false);
    } else {
      omitSidesets_ = false;
    }
  }
  
  {
    const char *temp = options_.retrieve("gvar");
    parse_variable_names(temp, &globalVarNames_);
  }

  {
    const char *temp = options_.retrieve("nvar");
    parse_variable_names(temp, &nodeVarNames_);
  }

  {
    const char *temp = options_.retrieve("evar");
    parse_variable_names(temp, &elemVarNames_);
  }

  {
    const char *temp = options_.retrieve("nsetvar");
    parse_variable_names(temp, &nsetVarNames_);
  }

  {
    const char *temp = options_.retrieve("ssetvar");
    parse_variable_names(temp, &ssetVarNames_);
  }

  if (options_.retrieve("disable_field_recognition")) {
    disableFieldRecognition_ = true;
  } else {
    disableFieldRecognition_ = false;
  }

  if (options_.retrieve("64-bit")) {
    ints64bit_ = true;
  }
  
  if (options_.retrieve("match_node_ids")) {
    matchNodeIds_ = true;
    matchNodeXYZ_ = false;
  } else {
    matchNodeIds_ = false;
  }
  
  if (options_.retrieve("match_node_coordinates")) {
    matchNodeXYZ_ = true;
    matchNodeIds_ = false;
  } else {
    matchNodeXYZ_ = false;
  }
  
#if 0
  if (options_.retrieve("match_elem_ids")) {
    matchElemIds_ = true;
    matchElemXYZ_ = false;
  } else {
    matchElemIds_ = false;
  }
  
  if (options_.retrieve("match_elem_coordinates")) {
    matchElemXYZ_ = true;
    matchElemIds_ = false;
  } else {
    matchElemXYZ_ = false;
  }
#endif
  
  return true;
}

bool SystemInterface::convert_nodes_to_nodesets(int part_number) const
{
  if (nodesetConvertParts_.empty())
    return false;
  else if (nodesetConvertParts_[0] == 0)
    return true;
  else
    return std::find(nodesetConvertParts_.begin(),
		     nodesetConvertParts_.end(),
		     part_number) != nodesetConvertParts_.end();
}

void SystemInterface::parse_step_option(const char *tokens)
{
  //: The defined formats for the count attribute are:<br>
  //:  <ul>
  //:    <li><missing> -- default -- 1 <= count <= oo  (all steps)</li>
  //:    <li>"X"                  -- X <= count <= X  (just step X). If X == LAST, last step only</li>
  //:    <li>"X:Y"                -- X to Y by 1</li>
  //:    <li>"X:"                 -- X to oo by 1</li>
  //:    <li>":Y"                 -- 1 to Y by 1</li>
  //:    <li>"::Z"                -- 1 to oo by Z</li>
  //:    <li>"LAST"               -- last step only</li>
  //:  </ul>
  //: The count and step must always be >= 0

  // Break into tokens separated by ":"

  // Default is given in constructor above...

  if (tokens != nullptr) {
    if (strchr(tokens, ':') != nullptr) {
      // The string contains a separator

      int vals[3];
      vals[0] = stepMin_;
      vals[1] = stepMax_;
      vals[2] = stepInterval_;

      int j=0;
      for (auto & val : vals) {
        // Parse 'i'th field
        char tmp_str[128];;
        int k=0;

        while (tokens[j] != '\0' && tokens[j] != ':') {
          tmp_str[k++] = tokens[j++];
        }

        tmp_str[k] = '\0';
        if (strlen(tmp_str) > 0)
          val = strtoul(tmp_str, nullptr, 0);

        if (tokens[j++] == '\0') {
          break; // Reached end of string
        }
      }
      stepMin_      = abs(vals[0]);
      stepMax_      = abs(vals[1]);
      stepInterval_ = abs(vals[2]);
    } else if (case_strcmp("LAST", tokens) == 0) {
      stepMin_ = stepMax_ = -1;
    } else {
      // Does not contain a separator, min == max
      stepMin_ = stepMax_ = strtol(tokens, nullptr, 0);
    }
  }
}

void SystemInterface::show_version()
{
  std::cout << "EJoin" << "\n"
	    << "\t(A code for merging Exodus II databases; with or without results data.)\n"
	    << "\t(Version: " << qainfo[2] << ") Modified: " << qainfo[1] << '\n';
}

namespace {
  std::string LowerCase(const std::string &name)
  {
    std::string s = name;
    std::transform (s.begin(), s.end(),    // source
		    s.begin(),             // destination
		    ::tolower);            // operation
    return s;
  }

  typedef std::vector<std::string> StringVector;
  bool string_id_sort(const std::pair<std::string,int> &t1,
		      const std::pair<std::string,int> &t2)
  {
    return t1.first < t2.first || (!(t2.first < t1.first) &&
				   t1.second < t2.second);
  }

  void parse_variable_names(const char *tokens, StringIdVector *variable_list)
  {
    // Break into tokens separated by ","
    if (tokens != nullptr) {
      std::string token_string(tokens);
      StringVector var_list = SLIB::tokenize(token_string, ",");
    
      // At this point, var_list is either a single string, or a string
      // separated from 1 or more block ids with ":" delimiter.
      // For example, sigxx:1:10:100 would indicate that the variable
      // "sigxx" should be written only for blocks with id 1, 10, and
      // 100.  "sigxx" would indicate that the variable should be
      // written for all blocks.
      auto I = var_list.begin();
      while (I != var_list.end()) {
	StringVector name_id = SLIB::tokenize(*I, ":");
	std::string var_name = LowerCase(name_id[0]);
	if (name_id.size() == 1) {
	  (*variable_list).push_back(std::make_pair(var_name,0));
	} else {
	  for (size_t i=1; i < name_id.size(); i++) {
	    // Convert string to integer...
	    int id = strtoul(name_id[i].c_str(), nullptr, 0);
	    (*variable_list).push_back(std::make_pair(var_name,id));
	  }
	}
	++I;
      }
      // Sort the list...
      std::sort(variable_list->begin(), variable_list->end(), string_id_sort);
    }
  }

  void parse_offset(const char *tokens, vector3d *offset)
  {
    // Break into tokens separated by ","
    if (tokens != nullptr) {
      std::string token_string(tokens);
      StringVector var_list = SLIB::tokenize(token_string, ",");
    
      // At this point, var_list should contain 1,2,or 3 strings
      // corresponding to the x, y, and z coordinate offsets.
      if (var_list.size() != 3) {
	std::cerr << "ERROR: Incorrect number of offset components specified--3 required.\n\n";
	offset->x = offset->y = offset->z = 0.0;
	return;
      }

      std::string offx = var_list[0];
      std::string offy = var_list[1];
      std::string offz = var_list[2];
      double x = strtod(offx.c_str(), nullptr);
      double y = strtod(offy.c_str(), nullptr);
      double z = strtod(offz.c_str(), nullptr);
      
      offset->x = x;
      offset->y = y;
      offset->z = z;
    }
  }

  void parse_integer_list(const char *tokens, std::vector<int> *list)
  {
    // Break into tokens separated by ","
    if (tokens != nullptr) {
      if (LowerCase(tokens) == "all") {
	(*list).push_back(0);
	return;
      }

      std::string token_string(tokens);
      StringVector part_list = SLIB::tokenize(token_string, ",");
    
      auto I = part_list.begin();
      while (I != part_list.end()) {
	int id = strtol((*I).c_str(), nullptr, 0);
	(*list).push_back(id);
	++I;
      }
    }
  }

  void parse_part_list(const char *tokens, std::vector<int> *list)
  {
    // Break into tokens separated by ","
    // Tokens will be of the form "p$" or "all"
    if (tokens != nullptr) {
      if (LowerCase(tokens) == "all") {
	(*list).push_back(0);
	return;
      }

      std::string token_string(tokens);
      StringVector part_list = SLIB::tokenize(token_string, ",");
    
      auto I = part_list.begin();
      while (I != part_list.end()) {
	std::string part = *I;
	if (part[0] == 'p' || part[0] == 'P') {
	  std::string part_id(part,1);
	  int part_num = strtoul(part_id.c_str(), nullptr, 0);
	  list->push_back(part_num);
	} else {
	  std::cerr << "ERROR: Bad syntax (" << part << ") specifying part number. Use 'p'+ part_number\n"
		    << "       For example -info_records p1,p2,p7\n";
	  exit(EXIT_FAILURE);
	}
	++I;
      }
    }
  }

  void parse_omissions(const char *tokens, Omissions *omissions,
		       const std::string &basename, bool require_ids)
  {
    //	to Omit block id 1,3,4 from part 1; block 2 3 4 from part 2;
    //	and block 8 from part5, specify
    // '-omit_blocks p1:1:3:4,p2:2:3:4,p5:8'

    // Break into tokens separated by "," Each token will then be a
    // ":" separated list of blocks to be omitted for the specified
    // part.
    
    // If "require_ids" is true, then there must be at least one id
    // following the part specification.  If false, then it is OK to
    // just specify a part number and all entities (typically nset or
    // sset) will be omitted on that part.
    
    if (tokens == nullptr)
      return;
    
    std::string token_string(tokens);
    StringVector part_block_list = SLIB::tokenize(token_string, ",");

    // Now, for each token in 'part_block_list', split by ":"
    // The result should be a string starting with 'p' followed by an
    // integer indicating the part followed by 1 or more strings which
    // are the block ids of the blocks to be omitted for that part.
    //
    // Parts are 1-based.  Store the results in the 'omissions' set as
    // the pair (part#, block_id).
    auto I = part_block_list.begin();
    while (I != part_block_list.end()) {
      StringVector part_block = SLIB::tokenize(*I, ":");
      if (part_block.empty() || (part_block[0][0] != 'p' && part_block[0][0] != 'P')) {
	std::cerr << "ERROR: Bad syntax specifying the part number.  Use 'p' + part number\n"
		  << "       For example -omit_blocks p1:1:2:3,p2:2:3:4\n";
	exit(EXIT_FAILURE);
      }
      if (require_ids && part_block.size() == 1) {
	std::cerr << "ERROR: No block ids were found following the part specification.\n"
		  << "       for part " << part_block[0] << "\n";
	exit(EXIT_FAILURE);
      }
      
      // Extract the part number...
      std::string part(part_block[0],1);
      int part_num = strtoul(part.c_str(), nullptr, 0) - 1;

      // If no blocks specified for a part, then omit all entities for
      // this part.  Since don't know how many entities there are,
      // store the id as '0' to signify all.
      if (part_block.size() == 1) {
	(*omissions)[part_num].push_back("ALL");
      } else {
	// Get the list of blocks to omit for this part...
	auto J = part_block.begin(); ++J; // Skip p#
	while (J != part_block.end()) {
	  std::string block = *J;
	  std::string name = basename + '_'+ block;
	  (*omissions)[part_num].push_back(name);
	  ++J;
	}
      }
      ++I;
    }
  }
}
