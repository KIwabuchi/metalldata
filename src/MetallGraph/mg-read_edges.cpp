// Copyright 2022 Lawrence Livermore National Security, LLC and other MetallData Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

/// \brief Implements distributed processing of a json file
///        based on the distributed YGM line parser.

#include "mg-common.hpp"

namespace xpr     = experimental;

namespace
{
const std::string METHOD_NAME  = "read_edges";
const std::string METHOD_DESC  = "Imports Json Data from files into the vertex container.";

using             ARG_EDGE_FILES_TYPE = std::vector<std::string>;
const std::string ARG_EDGE_FILES_NAME = "files";
const std::string ARG_EDGE_FILES_DESC = "A list of Json files that will be imported as edges.";
} // anonymous

int ygm_main(ygm::comm& world, int argc, char** argv)
{
  int             error_code = 0;
  clippy::clippy  clip{METHOD_NAME, METHOD_DESC};

  clip.member_of(MG_CLASS_NAME, "A " + MG_CLASS_NAME + " class");

  clip.add_required<ARG_EDGE_FILES_TYPE> (ARG_EDGE_FILES_NAME,  ARG_EDGE_FILES_DESC);
  clip.add_required_state<std::string>   (ST_METALL_LOCATION,   "Metall storage location");

  if (clip.parse(argc, argv, world)) { return 0; }

  try
  {
    using metall_manager = xpr::MetallJsonLines::metall_manager_type;

    const ARG_EDGE_FILES_TYPE   edgeFiles    = clip.get<ARG_EDGE_FILES_TYPE> (ARG_EDGE_FILES_NAME);
    const std::string           dataLocation = clip.get_state<std::string>(ST_METALL_LOCATION);
    metall_manager              mm{metall::open_only, dataLocation.data(), MPI_COMM_WORLD};
    xpr::MetallGraph            g{mm, world};
    const xpr::ImportSummary    summary      = g.readEdgeFiles(edgeFiles);

    if (world.rank() == 0)
    {
      clip.to_return(summary.asJson());
    }
  }
  catch (const std::exception& err)
  {
    error_code = 1;
    if (world.rank() == 0) clip.to_return(err.what());
  }

  return error_code;
}

