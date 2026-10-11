#include "odu_dmrs/controller.hpp"

namespace odu_dmrs {
namespace {
constexpr std::uint16_t kFullMask = (1u << 14) - 1u;
constexpr bool supported(std::uint16_t m) noexcept {
  return m == kMaskPos0 || m == kMaskPos1 || m == kMaskPos2 || m == kMaskPos3;
}
}

std::uint8_t symbol_count(std::uint16_t mask) noexcept {
  std::uint8_t count = 0;
  while (mask != 0) { mask = static_cast<std::uint16_t>(mask & (mask - 1u)); ++count; }
  return count;
}
std::uint16_t mask_for_code(std::uint8_t code) noexcept {
  switch (code) {
    case 1: return kMaskPos0;
    case 2: return kMaskPos1;
    case 3: return kMaskPos2;
    case 4: return kMaskPos3;
    default: return 0;
  }
}
const char* source_name(Source s) noexcept {
  switch (s) { case Source::Passthrough: return "passthrough";
    case Source::Forced: return "forced"; case Source::Cqi: return "cqi"; }
  return "unknown";
}
const char* status_name(Status s) noexcept {
  switch (s) { case Status::Ok: return "ok";
    case Status::InvalidGrant: return "invalid_grant";
    case Status::UnsupportedConfiguration: return "unsupported_configuration"; }
  return "unknown";
}
bool valid_configuration(const Configuration& c) noexcept {
  const auto mode = static_cast<unsigned>(c.mode);
  return mode <= static_cast<unsigned>(Mode::CqiPolicy) && c.forced_code >= 1 &&
         c.forced_code <= 4 && c.threshold_lo <= 15 && c.threshold_hi <= 15 &&
         c.threshold_lo < c.threshold_hi;
}
bool valid_grant(const GrantInput& g) noexcept {
  return g.cqi <= 15 && g.cdm_groups_no_data >= 1 && g.cdm_groups_no_data <= 2 &&
         g.nof_layers >= 1 && g.nof_layers <= 2 && g.nof_prbs > 0 &&
         g.existing_dmrs_mask != 0 && (g.existing_dmrs_mask & ~kFullMask) == 0;
}
std::uint32_t Controller::encode(Configuration c) noexcept {
  return static_cast<std::uint32_t>(c.mode) | (static_cast<std::uint32_t>(c.forced_code) << 2u) |
      (static_cast<std::uint32_t>(c.threshold_lo) << 5u) |
      (static_cast<std::uint32_t>(c.threshold_hi) << 9u);
}
Configuration Controller::decode(std::uint32_t w) noexcept {
  return Configuration{static_cast<Mode>(w & 0x3u), static_cast<std::uint8_t>((w >> 2u) & 0x7u),
      static_cast<std::uint8_t>((w >> 5u) & 0xFu), static_cast<std::uint8_t>((w >> 9u) & 0xFu)};
}
Controller::Controller(Configuration initial) : configuration_word_(encode(valid_configuration(initial) ? initial : Configuration{})) {}
bool Controller::configure(Configuration config) noexcept {
  if (!valid_configuration(config)) return false;
  configuration_word_.store(encode(config), std::memory_order_release);
  return true;
}
Configuration Controller::configuration() const noexcept {
  return decode(configuration_word_.load(std::memory_order_acquire));
}
Decision Controller::decide(const GrantInput& g) const noexcept {
  Decision out{};
  // On a rejected grant no resource mutation is performed.
  out.final_dmrs_mask = g.existing_dmrs_mask;
  if (!valid_grant(g)) return out;
  out.symbol_count = symbol_count(g.existing_dmrs_mask);
  out.dmrs_re_per_prb = static_cast<std::uint16_t>(6u * g.cdm_groups_no_data * out.symbol_count);
  // Guard: these lookup masks cannot represent arbitrary PDSCH/RRC settings.
  if (!supported(g.existing_dmrs_mask)) {
    out.status = Status::UnsupportedConfiguration;
    return out;
  }
  const Configuration config = configuration();
  if (config.mode == Mode::Passthrough) {
    out.status = Status::Ok;
    return out;
  }
  if (config.mode == Mode::Forced) {
    out.decided_code = config.forced_code;
    out.source = Source::Forced;
  } else {
    out.source = Source::Cqi;
    out.decided_code = g.cqi <= config.threshold_lo ? 4 :
        (g.cqi >= config.threshold_hi ? 2 : 3);
  }
  const std::uint16_t target = mask_for_code(out.decided_code);
  if (target == 0) return out; // Defensive fallback; never reached for valid config.
  if (target != g.existing_dmrs_mask) {
    out.changed = true;
    out.final_dmrs_mask = target;
    out.applied_additional_pos = static_cast<std::uint8_t>(out.decided_code - 1u);
    out.symbol_count = symbol_count(target);
    out.dmrs_re_per_prb = static_cast<std::uint16_t>(6u * g.cdm_groups_no_data * out.symbol_count);
  }
  out.status = Status::Ok;
  return out;
}
} // namespace odu_dmrs
