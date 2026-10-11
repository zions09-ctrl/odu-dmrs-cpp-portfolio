#pragma once
#include "odu_dmrs/controller.hpp"
#include <cstdint>

namespace odu_dmrs {
// Thin integration boundary. NO dependency on OCUDU headers here.
// A production adapter must map this result to the actual OCUDU DM-RS
// allocator before DCI/PDSCH commitment and cross-check the committed PDU.
struct AllocatorGrant {
  std::uint64_t slot{0};
  std::uint8_t cqi{15};
  std::uint16_t dmrs_mask{kMaskPos2};
  std::uint8_t cdm_groups_no_data{1};
  std::uint8_t layers{2};
  std::uint16_t prbs{51};
};

class SchedulerAdapter final {
 public:
  explicit SchedulerAdapter(const Controller& controller) : controller_(controller) {}
  Decision propose(const AllocatorGrant& g) const noexcept {
    return controller_.decide(GrantInput{g.slot, g.cqi, g.dmrs_mask,
      g.cdm_groups_no_data, g.layers, g.prbs});
  }
 private:
  const Controller& controller_;
};
} // namespace odu_dmrs
