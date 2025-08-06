#ifndef OPENMC_TALLIES_FILTER_TOFENERGYRATIO_H
#define OPENMC_TALLIES_FILTER_TOFENERGYRATIO_H

#include "openmc/span.h"
#include "openmc/tallies/filter.h"
#include "openmc/vector.h"

namespace openmc {

//==============================================================================
//! Bins the relative difference between particle actual energy and expected energy from time-of-flight.
//==============================================================================

class TofEnergyRatioFilter : public Filter {
public:
  //----------------------------------------------------------------------------
  // Constructors, destructors

  ~TofEnergyRatioFilter() = default;

  //----------------------------------------------------------------------------
  // Methods

  std::string type_str() const override { return "tofenergyratio"; }
  FilterType type() const override { return FilterType::TOFENERGYRATIO; }

  void from_xml(pugi::xml_node node) override;

  void get_all_bins(const Particle& p, TallyEstimator estimator,
    FilterMatch& match) const override;

  void to_statepoint(hid_t filter_group) const override;

  std::string text_label(int bin) const override;

  //----------------------------------------------------------------------------
  // Accessors

  const vector<double>& bins() const { return bins_; }
  void set_bins(span<const double> bins);

  double distance() const { return distance_; }
  void set_distance(double distance);

protected:
  //----------------------------------------------------------------------------
  // Data members

  vector<double> bins_;
  double distance_ {1.0};
};

} // namespace openmc
#endif // OPENMC_TALLIES_FILTER_TOFENERGYRATIO_H
