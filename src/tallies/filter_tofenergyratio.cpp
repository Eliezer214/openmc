#include "openmc/tallies/filter_tofenergyratio.h"
#include "openmc/constants.h"
#include "openmc/particle.h"
#include "openmc/search.h"
#include "openmc/xml_interface.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <fmt/core.h>

namespace openmc {

//==============================================================================
// from_xml
//==============================================================================
void TofEnergyRatioFilter::from_xml(pugi::xml_node node)
{
  auto bins = get_node_array<double>(node, "bins");
  set_bins(bins);

  std::string s = get_node_value(node, "distance");
  double d = std::stod(s);
  set_distance(d);
}

//==============================================================================
// set_bins
//==============================================================================
void TofEnergyRatioFilter::set_bins(span<const double> bins) {
    bins_.clear();
    bins_.reserve(bins.size());

    if (bins.size() < 2) {
        throw std::runtime_error{"TofEnergyRatioFilter: bins must contain at least two edges"};
    }
    for (size_t i = 0; i < bins.size(); ++i) {
        if (i > 0 && bins[i] <= bins[i - 1]) {
            throw std::runtime_error{"TofEnergyRatioFilter: bins must be strictly increasing"};
        }
        bins_.push_back(bins[i]);
    }
    n_bins_ = bins_.size() - 1;
}

//==============================================================================
// set_distance
//==============================================================================
void TofEnergyRatioFilter::set_distance(double distance)
{
  if (distance <= 0.0) {
    throw std::runtime_error{"distance must be positive"};
  }
  distance_ = distance;
}

//==============================================================================
// get_all_bins
//==============================================================================
void TofEnergyRatioFilter::get_all_bins(const Particle& p, TallyEstimator estimator, FilterMatch& match) const {
    double E = (estimator == TallyEstimator::TRACKLENGTH) ? p.E_last() : p.E();
    double t = (estimator == TallyEstimator::TRACKLENGTH) ? p.time_last() : p.time();

    double v = distance_ / t;
    double beta = v / C_LIGHT;
    double gamma = 1.0 / std::sqrt(1.0 - beta * beta);
    double E_expected = (gamma - 1.0) * MASS_NEUTRON_EV;

    double mismatch = (E - E_expected) / E_expected;

    
    if (mismatch >= bins_.front() && mismatch < bins_.back()) {
        auto bin = lower_bound_index(bins_.begin(), bins_.end(), mismatch);
        match.bins_.push_back(bin);
        match.weights_.push_back(1.0);
    }
}


//==============================================================================
// to_statepoint
//==============================================================================
void TofEnergyRatioFilter::to_statepoint(hid_t filter_group) const
{
  Filter::to_statepoint(filter_group);
  write_dataset(filter_group, "bins", bins_);
  write_dataset(filter_group, "distance", distance_);
}

//==============================================================================
// text_label
//==============================================================================
std::string TofEnergyRatioFilter::text_label(int bin) const {
    return fmt::format("TOF mismatch [{}, {})", bins_[bin], bins_[bin + 1]);
}

} // namespace openmc
