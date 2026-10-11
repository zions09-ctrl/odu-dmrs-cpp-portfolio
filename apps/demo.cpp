#include "odu_dmrs/scheduler_adapter.hpp"
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>

namespace {
std::string bits(std::uint16_t mask) {
  std::string out;
  for (int symbol=13; symbol>=0; --symbol) out.push_back((mask & (1u << symbol)) ? '1' : '0');
  return out;
}
}
int main(int argc, char** argv) {
  std::size_t slots = 24;
  if (argc > 1) {
    try { slots = static_cast<std::size_t>(std::stoul(argv[1])); }
    catch (...) { std::cerr << "usage: dmrs_demo [slots 1..100000]\n"; return 2; }
  }
  if (slots == 0 || slots > 100000) return 2;
  odu_dmrs::Controller controller;
  controller.configure({odu_dmrs::Mode::CqiPolicy, 3, 6, 13});
  odu_dmrs::SchedulerAdapter adapter(controller);
  std::cerr << "STANDALONE DEMO: synthetic CQI; no OCUDU/PHY/OTA execution.\n";
  std::cout << "slot,cqi,source,code,applied_pos,changed,mask,symbols,re_per_prb,ctrl_ns,status\n";
  for (std::size_t slot=0; slot<slots; ++slot) {
    // Simulated remote update at slot N/2; not a WebSocket experiment.
    if (slot == slots/2) controller.configure({odu_dmrs::Mode::Forced, 4, 6, 13});
    odu_dmrs::AllocatorGrant g;
    g.slot = slot;
    g.cqi = static_cast<std::uint8_t>((slot * 7 + 3) % 16);
    const auto t0 = std::chrono::steady_clock::now();
    const auto result = adapter.propose(g);
    const auto t1 = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    std::cout << slot << ',' << unsigned(g.cqi) << ',' << odu_dmrs::source_name(result.source)
        << ',' << unsigned(result.decided_code) << ',' << unsigned(result.applied_additional_pos)
        << ',' << result.changed << ',' << bits(result.final_dmrs_mask) << ','
        << unsigned(result.symbol_count) << ',' << result.dmrs_re_per_prb << ','
        << elapsed << ',' << odu_dmrs::status_name(result.status) << '\n';
  }
}
