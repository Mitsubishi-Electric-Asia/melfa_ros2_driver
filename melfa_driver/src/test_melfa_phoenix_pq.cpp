// COPYRIGHT (C) 2026 Mitsubishi Electric Corporation & 8b-is (https://8b.is)
//
// Licensed under the Apache License, Version 2.0 (the "License");

#include "melfa_driver/melfa_phoenix_lattice_pq.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

int main()
{
  using namespace MelfaEthernet::phoenix_pq;

  // 1. Verify 5-state Laser Phoenix Interference Lattice
  assert(
    classify_phoenix_interference(static_cast<int32_t>(kPhoenixLatticeAnchorsQ32[0])) ==
    PhoenixInterferenceState::Bound);
  assert(
    classify_phoenix_interference(static_cast<int32_t>(kPhoenixLatticeAnchorsQ32[1])) ==
    PhoenixInterferenceState::Related);
  assert(
    classify_phoenix_interference(static_cast<int32_t>(kPhoenixLatticeAnchorsQ32[2])) ==
    PhoenixInterferenceState::Independent);
  assert(
    classify_phoenix_interference(static_cast<int32_t>(kPhoenixLatticeAnchorsQ32[3])) ==
    PhoenixInterferenceState::Contrasting);
  assert(
    classify_phoenix_interference(static_cast<int32_t>(kPhoenixLatticeAnchorsQ32[4])) ==
    PhoenixInterferenceState::Conflicting);

  // 2. Verify MelfaPhoenixPulsePll & Marine Servo-Jitter Gate
  MelfaPhoenixPulsePll pll(64);
  int64_t raw_err_sum = 0;
  int64_t pll_err_sum = 0;
  bool saw_clip = false;

  for (int step = 0; step < 50; ++step) {
    const int32_t gt_p1 = 10000 + step * 200;
    const int32_t jitter = (step >= 15 && step < 35) ? ((step % 2 == 0) ? 120 : -120) : 0;
    PULSE raw{gt_p1 + jitter, gt_p1 * 2, gt_p1 * 3, 0, 0, 0, 0, 0};
    const auto res = pll.step(raw);
    if (res.jitter_clipped) saw_clip = true;
    if (step >= 15 && step < 35) {
      raw_err_sum += std::abs(raw.p1 - gt_p1);
      pll_err_sum += std::abs(res.locked_pulse.p1 - gt_p1);
    }
  }
  assert(saw_clip);
  assert(pll_err_sum < raw_err_sum);

  // 3. Verify Bessel-Overhauser Catmull-Rom 8-axis PULSE UDP packet dropout recovery
  std::vector<PULSE> gt_traj(20);
  std::vector<bool> valid_mask(20, true);
  for (int i = 0; i < 20; ++i) {
    // Exact quadratic + linear 8-axis motor pulse trajectory
    const int32_t q = 1000 + 3 * i * i + 50 * i;
    gt_traj[i] = PULSE{q, q * 2, -q, q + 500, q / 2, -2 * q, q + 100, 0};
  }

  std::vector<PULSE> corrupted = gt_traj;
  for (size_t idx : {6U, 7U, 8U, 9U}) {
    valid_mask[idx] = false;
    corrupted[idx] = PULSE{0, 0, 0, 0, 0, 0, 0, 0};
  }

  const size_t restored = restore_dropped_mxt_pulses_catmull_rom_pq(corrupted, valid_mask);
  assert(restored == 4U);

  for (size_t i = 0; i < 20; ++i) {
    assert(std::abs(corrupted[i].p1 - gt_traj[i].p1) <= 1);
    assert(std::abs(corrupted[i].p2 - gt_traj[i].p2) <= 1);
    assert(std::abs(corrupted[i].p3 - gt_traj[i].p3) <= 1);
  }

  std::cout << "[PASS] All MELFA Laser Phoenix Lattice & Catmull-Rom p/q tests passed!\n";
  return 0;
}
