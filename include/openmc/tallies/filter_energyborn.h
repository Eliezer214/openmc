#ifndef OPENMC_TALLIES_FILTER_ENERGYBORN_H
#define OPENMC_TALLIES_FILTER_ENERGYBORN_H

#include "openmc/tallies/filter_energy.h"
#include "openmc/span.h"
#include <string>

namespace openmc {

class EnergyBornFilter : public EnergyFilter {
public:
  ~EnergyBornFilter() override;  // key function for vtable

  std::string type_str() const override { return "energyborn"; }
  FilterType   type()     const override { return FilterType::ENERGYBORN; }

  void from_xml(pugi::xml_node node) override;
  void get_all_bins(const Particle& p, TallyEstimator estimator,
                    FilterMatch& match) const override;
  void to_statepoint(hid_t filter_group) const override;
  std::string text_label(int bin) const override;
};

} // namespace openmc

#endif // OPENMC_TALLIES_FILTER_ENERGYBORN_H