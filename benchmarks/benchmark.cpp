#include "odu_dmrs/controller.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {
double percentile(const std::vector<std::int64_t>& values, double q) {
  const double index = (values.size() - 1u) * q;
  const auto a = static_cast<std::size_t>(std::floor(index));
  const auto b = std::min(a+1,values.size()-1);
  return values[a] + (values[b]-values[a]) * (index-a);
}
}
int main(int argc, char** argv) {
  std::size_t n = 100000;
  if (argc>1) {
    try { n = std::stoul(argv[1]); }
    catch (...) { std::cerr << "usage: dmrs_benchmark [iterations 100..2000000]\n"; return 2; }
  }
  if (n<100 || n>2000000) return 2;
  odu_dmrs::Controller controller({odu_dmrs::Mode::CqiPolicy,3,6,13});
  std::vector<odu_dmrs::GrantInput> inputs(n);
  for (std::size_t i=0;i<n;++i) {
    inputs[i].slot=i;
    inputs[i].cqi=static_cast<std::uint8_t>((i*7u+3u)%16u);
    inputs[i].existing_dmrs_mask=odu_dmrs::kMaskPos2;
  }
  volatile std::uint64_t checksum=0;
  for (std::size_t i=0;i<10000;++i) checksum=checksum+controller.decide(inputs[i%n]).dmrs_re_per_prb;
  std::vector<std::int64_t> times(n);
  const auto t_start=std::chrono::steady_clock::now();
  for (std::size_t i=0;i<n;++i) {
    const auto t0=std::chrono::steady_clock::now();
    const auto result=controller.decide(inputs[i]);
    const auto t1=std::chrono::steady_clock::now();
    checksum=checksum+result.dmrs_re_per_prb;
    times[i]=std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
  }
  const auto t_end=std::chrono::steady_clock::now();
  std::sort(times.begin(),times.end());
  const double mean=std::accumulate(times.begin(),times.end(),0.0)/times.size();
  const auto total=std::chrono::duration_cast<std::chrono::nanoseconds>(t_end-t_start).count();
  std::cout << std::fixed << std::setprecision(2);
  std::cout << "{\n"
    << "  \"test_type\": \"standalone_host_loopback_benchmark\",\n"
    << "  \"scope\": \"controller_decide_only_including_clock_calls\",\n"
    << "  \"warning\": \"not_OCUDU_not_PHY_not_WCET_not_deadline_proof\",\n"
    << "  \"samples\": " << n << ",\n"
    << "  \"mean_ns\": " << mean << ",\n"
    << "  \"p50_ns\": " << percentile(times,0.5) << ",\n"
    << "  \"p99_ns\": " << percentile(times,0.99) << ",\n"
    << "  \"p999_ns\": " << percentile(times,0.999) << ",\n"
    << "  \"max_ns\": " << times.back() << ",\n"
    << "  \"elapsed_total_ns\": " << total << ",\n"
    << "  \"checksum\": " << checksum << "\n}\n";
  return 0;
}
