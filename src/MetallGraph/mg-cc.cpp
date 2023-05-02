// Copyright 2022 Lawrence Livermore National Security, LLC and other MetallData Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

/// \brief Implements MetallGraph constructor (__init__).

#include "mg-common.hpp"

namespace xpr     = experimental;

namespace
{
const std::string METHOD_NAME      = "connected_components";
const std::string METHOD_DOCSTRING = "Computes connected components..";
} // anonymous

std::size_t countLines( bool skip,
                        bool ignoreFilter,
                        xpr::MetallJsonLines& lines,
                        std::size_t rank,
                        clippy::clippy& clip,
                        std::string_view selector
                      )
{
  if (skip) return 0;
  if (ignoreFilter) return lines.count();

  return lines.filter(filter(rank, clip, selector)).count();
}

int ygm_main(ygm::comm& world, int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, METHOD_DOCSTRING};

  clip.member_of(MG_CLASS_NAME, "A " + MG_CLASS_NAME + " class");
  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");

  if (clip.parse(argc, argv, world)) { return 0; }

  try
  {
    using metall_manager = xpr::MetallJsonLines::metall_manager_type;

    const std::string   dataLocation = clip.get_state<std::string>(ST_METALL_LOCATION);
    metall_manager      mm{metall::open_read_only, dataLocation.data(), MPI_COMM_WORLD};
    xpr::MetallGraph    g{mm, world};
    const std::size_t   res = g.connectedComponents( filter(world.rank(), clip, NODES_SELECTOR),
                                                     filter(world.rank(), clip, EDGES_SELECTOR)
                                                   );

    if (world.rank() == 0)
    {
      clip.to_return(res);
    }
  }
  catch (const std::exception& err)
  {
    error_code = 1;
    if (world.rank() == 0) clip.to_return(err.what());
  }
  catch (...)
  {
    error_code = 1;
    if (world.rank() == 0) clip.to_return("unhandled, unknown exception");
  }

  return error_code;
}


