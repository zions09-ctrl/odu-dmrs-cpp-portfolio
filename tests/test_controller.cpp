#include "odu_dmrs/scheduler_adapter.hpp"
#include <atomic>
#include <cstdint>
#include <iostream>
#include <thread>

namespace {
int failures=0, assertions=0;
void check(bool value,const char* msg){++assertions;if(!value){++failures;std::cerr<<"FAIL "<<msg<<'\n';}}
}
int main(){
  using namespace odu_dmrs;
  check(!valid_configuration({Mode::CqiPolicy,3,13,6}),"reject reversed thresholds");
  check(!valid_configuration({Mode::Forced,0,6,13}),"reject unsupported code");
  check(!valid_configuration({Mode::Forced,5,6,13}),"reject code 5");
  check(mask_for_code(0)==0,"no code0 target mask");
  check(mask_for_code(5)==0,"no code5 target mask");
  constexpr std::uint16_t expected[]={kMaskPos0,kMaskPos1,kMaskPos2,kMaskPos3};
  for (std::uint8_t i=1;i<=4;++i) {
    check(mask_for_code(i)==expected[i-1],"mask table");
    check(symbol_count(expected[i-1])==i,"mask popcount");
  }
  Controller ctl;
  auto g=GrantInput{};
  auto x=ctl.decide(g);
  check(x.status==Status::Ok && x.decided_code==0,"passthrough\n");
  check(x.applied_additional_pos==kNoActuation && !x.changed,"passthrough no actuation");
  check(ctl.configure({Mode::Forced,3,6,13}),"forced configure");
  x=ctl.decide(g);
  check(x.decided_code==3 && !x.changed && x.applied_additional_pos==255,"same mask no-op");
  for (std::uint8_t code=1;code<=4;++code) {
    check(ctl.configure({Mode::Forced,code,6,13}),"configure forced");
    x=ctl.decide(g);
    check(x.status==Status::Ok && x.decided_code==code,"forced output");
    check(x.final_dmrs_mask==expected[code-1],"forced mask");
    check(x.symbol_count==code,"forced symbol count");
    check(x.dmrs_re_per_prb==6*code,"RE accounting for CDM=1");
    check(x.changed==(code!=3),"actual mutation only if differs");
    check(x.applied_additional_pos==(code==3 ? 255 : code-1),"actual position or no-op");
  }
  check(ctl.configure({Mode::CqiPolicy,3,6,13}),"cqi configure");
  for (std::uint8_t cqi=0;cqi<=15;++cqi) {
    g.cqi=cqi;x=ctl.decide(g);
    const auto expected_code= cqi<=6 ? 4u : (cqi>=13 ? 2u : 3u);
    check(x.decided_code==expected_code,"cqi policy boundaries");
  }
  g.cqi=15;g.cdm_groups_no_data=2; x=ctl.decide(g);
  check(x.dmrs_re_per_prb==24,"two CDM group accounting");
  g.cdm_groups_no_data=0;check(ctl.decide(g).status==Status::InvalidGrant,"reject CDM0");
  g.cdm_groups_no_data=1;g.cqi=16;check(ctl.decide(g).status==Status::InvalidGrant,"reject CQI16");
  g.cqi=15;g.existing_dmrs_mask=0b00000000000001;
  x=ctl.decide(g);check(x.status==Status::UnsupportedConfiguration && !x.changed,"unsupported mask fallback");
  g.existing_dmrs_mask=static_cast<std::uint16_t>(1u<<14);
  check(ctl.decide(g).status==Status::InvalidGrant,"reject out of 14 symbols");
  g.existing_dmrs_mask=kMaskPos2;g.nof_layers=3;
  check(ctl.decide(g).status==Status::InvalidGrant,"reject unsupported number layers");
  g.nof_layers=2;
  SchedulerAdapter adapter(ctl);
  check(adapter.propose(AllocatorGrant{}).status==Status::Ok,"adapter boundary");
  // Readers must never observe a torn config (e.g. lo=13, hi=6).
  std::atomic<bool> done{false};std::atomic<int> torn{0};
  std::thread writer([&]{for(int i=0;i<20000;++i){
    ctl.configure((i&1) ? Configuration{Mode::Forced,1,2,13} : Configuration{Mode::CqiPolicy,4,6,12});
    }done.store(true,std::memory_order_release);});
  std::thread reader([&]{while(!done.load(std::memory_order_acquire)){
    const auto c=ctl.configuration();
    if (!valid_configuration(c)) torn.fetch_add(1,std::memory_order_relaxed);
    const auto d=ctl.decide(g);
    if (d.status != Status::Ok || d.final_dmrs_mask==0) torn.fetch_add(1,std::memory_order_relaxed);
  }});
  writer.join();reader.join();
  check(torn.load()==0,"atomic configuration snapshot under concurrency");
  std::cout<<"controller_tests assertions="<<assertions<<" failures="<<failures<<'\n';
  return failures==0?0:1;
}
