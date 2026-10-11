#pragma once

#include <atomic>
#include <cstdint>

namespace odu_dmrs {
// This is a clean-room *standalone* controller, NOT the recovered OCUDU source.
// Maps are scoped to the 14-symbol, Type-A, single-frontload test configuration.
constexpr std::uint16_t kMaskPos0 = 0b00100000000000;
constexpr std::uint16_t kMaskPos1 = 0b00100000000100;
constexpr std::uint16_t kMaskPos2 = 0b00100001000100;
constexpr std::uint16_t kMaskPos3 = 0b00100100100100;
constexpr std::uint8_t kNoActuation = 255;

enum class Mode : std::uint8_t { Passthrough=0, Forced=1, CqiPolicy=2 };
enum class Status : std::uint8_t { Ok, InvalidGrant, UnsupportedConfiguration };
enum class Source : std::uint8_t { Passthrough=0, Forced=1, Cqi=2 };

struct Configuration {
  Mode mode{Mode::Passthrough};
  std::uint8_t forced_code{3}; // 1..4
  std::uint8_t threshold_lo{6};
  std::uint8_t threshold_hi{13};
};

struct GrantInput {
  std::uint64_t slot{0};
  std::uint8_t cqi{0};                  // 0..15
  std::uint16_t existing_dmrs_mask{kMaskPos2};
  std::uint8_t cdm_groups_no_data{1};  // 1..2 in this demo
  std::uint8_t nof_layers{2};          // 1..2 in this demo
  std::uint16_t nof_prbs{51};
};

struct Decision {
  Status status{Status::InvalidGrant};
  Source source{Source::Passthrough};
  std::uint8_t decided_code{0};           // 0: default, 1..4: pos0..3
  std::uint8_t applied_additional_pos{kNoActuation};
  std::uint16_t final_dmrs_mask{0};
  std::uint8_t symbol_count{0};
  std::uint16_t dmrs_re_per_prb{0};
  bool changed{false};
};

bool valid_configuration(const Configuration& config) noexcept;
bool valid_grant(const GrantInput& grant) noexcept;
std::uint8_t symbol_count(std::uint16_t mask) noexcept;
std::uint16_t mask_for_code(std::uint8_t code) noexcept;
const char* source_name(Source source) noexcept;
const char* status_name(Status status) noexcept;

class Controller final {
 public:
  // One atomic store; the slot reader observes the entire config consistently.
  explicit Controller(Configuration initial = {});
  bool configure(Configuration config) noexcept;
  Configuration configuration() const noexcept;
  // Bounded, allocation-free, lock-free hot path; does NOT mutate gNB grants.
  Decision decide(const GrantInput& grant) const noexcept;

 private:
  static std::uint32_t encode(Configuration config) noexcept;
  static Configuration decode(std::uint32_t config) noexcept;
  std::atomic<std::uint32_t> configuration_word_;
};
} // namespace odu_dmrs
