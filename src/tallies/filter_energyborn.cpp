#include "openmc/tallies/filter_energyborn.h"
#include "openmc/search.h"        // for lower_bound_index
#include "openmc/xml_interface.h" // for get_node_array
#include "openmc/mgxs_interface.h"// for data::mg
#include <fmt/core.h>

namespace openmc {

//------------------------------------------------------------------------------
// Out-of-line destructor: ensures the vtable is emitted here.
//------------------------------------------------------------------------------
EnergyBornFilter::~EnergyBornFilter() = default;

//==============================================================================
// Read bins from XML and pass to base class
//==============================================================================
void EnergyBornFilter::from_xml(pugi::xml_node node)
{
  auto bins = get_node_array<double>(node, "bins");
  this->set_bins(bins);  // calls EnergyFilter::set_bins()
}

//==============================================================================
// Bin the “born” energy for each particle
//==============================================================================
void EnergyBornFilter::get_all_bins(
  const Particle& p, TallyEstimator estimator, FilterMatch& match) const
{
  // If MG and bins match transport groups, use g-index directly
  if (p.g() != C_NONE && matches_transport_groups_) {
    int idx = (estimator == TallyEstimator::TRACKLENGTH ? p.g() : p.g_last());
    match.bins_.push_back(data::mg.num_energy_groups_ - idx - 1);
    match.weights_.push_back(1.0);

  } else {
    // Otherwise, use the energy at birth
    auto E = p.E_born();
    if (E >= bins_.front() && E <= bins_.back()) {
      auto bin = lower_bound_index(bins_.begin(), bins_.end(), E);
      match.bins_.push_back(bin);
      match.weights_.push_back(1.0);
    }
  }
}

//==============================================================================
// Write bins to statepoint
//==============================================================================
void EnergyBornFilter::to_statepoint(hid_t filter_group) const
{
  Filter::to_statepoint(filter_group);
  write_dataset(filter_group, "bins", bins_);
}

//==============================================================================
// Provide a human-readable label
//==============================================================================
std::string EnergyBornFilter::text_label(int bin) const
{
  return fmt::format("Born Energy [{}, {})", bins_[bin], bins_[bin + 1]);
}

} // namespace openmc