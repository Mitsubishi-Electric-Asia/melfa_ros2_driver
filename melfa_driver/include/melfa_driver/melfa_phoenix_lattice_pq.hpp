// COPYRIGHT (C) 2026 Mitsubishi Electric Corporation & 8b-is (https://8b.is)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __MELFA_PHOENIX_LATTICE_PQ_HPP__
#define __MELFA_PHOENIX_LATTICE_PQ_HPP__

#include "rt_exc_def.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace MelfaEthernet::phoenix_pq
{

/// @brief Canonical Golden Ratio rational convergent p/q = F_19 / F_18 = 4181 / 2584.
inline constexpr int64_t kPhiP = 4181;
inline constexpr int64_t kPhiQ = 2584;

/// @brief Default MELFA motor encoder pulses per revolution (2^17 = 131,072 pulses/rev).
inline constexpr int64_t kDefaultPulsesPerRev = 131072;

/**
 * @brief 5-state Laser Phoenix Interference Lattice on Z_{2^32}.
 */
enum class PhoenixInterferenceState : uint8_t {
  Bound = 0,        //   0 deg (|dtheta| < 22.5 deg)
  Related = 1,      //  45 deg (22.5 deg <= |dtheta| < 67.5 deg)
  Independent = 2,  //  90 deg (67.5 deg <= |dtheta| < 112.5 deg)
  Contrasting = 3,  // 135 deg (112.5 deg <= |dtheta| < 157.5 deg)
  Conflicting = 4   // 180 deg (|dtheta| >= 157.5 deg)
};

inline constexpr std::array<uint32_t, 5> kPhoenixLatticeAnchorsQ32 = {
  0x00000000U,  //   0.0 deg
  0x20000000U,  //  45.0 deg
  0x40000000U,  //  90.0 deg
  0x60000000U,  // 135.0 deg
  0x80000000U   // 180.0 deg
};

[[nodiscard]] inline PhoenixInterferenceState classify_phoenix_interference(
  int32_t signed_delta_q32) noexcept
{
  const uint32_t abs_d =
    (signed_delta_q32 < 0)
      ? static_cast<uint32_t>(-static_cast<int64_t>(signed_delta_q32))
      : static_cast<uint32_t>(signed_delta_q32);
  if (abs_d < 0x10000000U) return PhoenixInterferenceState::Bound;
  if (abs_d < 0x30000000U) return PhoenixInterferenceState::Related;
  if (abs_d < 0x50000000U) return PhoenixInterferenceState::Independent;
  if (abs_d < 0x70000000U) return PhoenixInterferenceState::Contrasting;
  return PhoenixInterferenceState::Conflicting;
}

[[nodiscard]] inline std::array<int32_t, 8> pulse_to_array(const PULSE & p) noexcept
{
  return {p.p1, p.p2, p.p3, p.p4, p.p5, p.p6, p.p7, p.p8};
}

[[nodiscard]] inline PULSE array_to_pulse(const std::array<int32_t, 8> & a) noexcept
{
  return PULSE{a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]};
}

/**
 * @brief Maps a motor encoder pulse count to the 32-bit cyclic phase group Z_{2^32}.
 */
[[nodiscard]] inline uint32_t pulse_to_phase_q32(
  int32_t pulse, int64_t pulses_per_rev = kDefaultPulsesPerRev) noexcept
{
  if (pulses_per_rev <= 0) return 0U;
  int64_t rem = static_cast<int64_t>(pulse) % pulses_per_rev;
  if (rem < 0) rem += pulses_per_rev;
  const uint64_t num =
    (static_cast<uint64_t>(rem) << 32) + static_cast<uint64_t>(pulses_per_rev / 2);
  return static_cast<uint32_t>(num / static_cast<uint64_t>(pulses_per_rev));
}

/**
 * @brief Zero-float 8-axis MELFA Motor Pulse Phase-Locked Loop with Marine Servo-Jitter Gate.
 *
 * Smooths Ethernet UDP arrival jitter and encoder quantization noise at CR800 3.5ms / CR750 7.11ms
 * control rates while tracking multi-turn motor pulse trajectories in 64-bit integer arithmetic.
 */
class MelfaPhoenixPulsePll
{
public:
  struct StepResult
  {
    PULSE locked_pulse;
    uint16_t coherence_q8;  // [0, 256]
    PhoenixInterferenceState worst_axis_state;
    bool jitter_clipped;
  };

  explicit MelfaPhoenixPulsePll(int64_t jitter_ref_pulses = 64) noexcept
  : jitter_ref_pulses_(jitter_ref_pulses > 0 ? jitter_ref_pulses : 1)
  {
  }

  [[nodiscard]] StepResult step(const PULSE & raw_pulse) noexcept
  {
    const auto raw = pulse_to_array(raw_pulse);

    if (!initialized_) {
      initialized_ = true;
      for (size_t ax = 0; ax < 8; ++ax) {
        locked_pulses_[ax] = raw[ax];
        prev_raw_pulses_[ax] = raw[ax];
      }
      return StepResult{raw_pulse, 256, PhoenixInterferenceState::Bound, false};
    }

    std::array<int64_t, 8> raw_vel{};
    for (size_t ax = 0; ax < 8; ++ax) {
      raw_vel[ax] = static_cast<int64_t>(raw[ax]) - prev_raw_pulses_[ax];
      prev_raw_pulses_[ax] = raw[ax];
    }

    if (!has_velocity_) {
      has_velocity_ = true;
      for (size_t ax = 0; ax < 8; ++ax) {
        vel_pulses_[ax] = raw_vel[ax];
        prev_raw_vel_[ax] = raw_vel[ax];
        locked_pulses_[ax] = raw[ax];
      }
      return StepResult{raw_pulse, 256, PhoenixInterferenceState::Bound, false};
    }

    // Compute max second-order acceleration jitter across all 8 MELFA axes
    uint64_t max_accel = 0;
    for (size_t ax = 0; ax < 8; ++ax) {
      const int64_t diff = raw_vel[ax] - prev_raw_vel_[ax];
      prev_raw_vel_[ax] = raw_vel[ax];
      const uint64_t abs_diff = static_cast<uint64_t>(diff < 0 ? -diff : diff);
      if (abs_diff > max_accel) max_accel = abs_diff;
    }

    jitter_ring_[jitter_idx_] = max_accel;
    jitter_idx_ = (jitter_idx_ + 1) & 7U;
    if (jitter_count_ < 8U) ++jitter_count_;

    uint64_t sum_jitter = 0;
    for (size_t i = 0; i < jitter_count_; ++i) sum_jitter += jitter_ring_[i];
    const uint64_t mean_jitter = sum_jitter / jitter_count_;

    const uint64_t penalty =
      (mean_jitter * 256ULL) / static_cast<uint64_t>(jitter_ref_pulses_);
    const uint16_t coherence_q8 =
      (penalty >= 256ULL) ? 0U : static_cast<uint16_t>(256ULL - penalty);

    // Marine Salience Gate: if coherence < 0.35 (90 / 256), halve innovation gain
    const bool jitter_clipped = (coherence_q8 < 90U);
    const int64_t alpha_num = jitter_clipped ? (kPhiQ / 2) : kPhiQ;
    const int64_t beta_num = jitter_clipped ? ((kPhiP - kPhiQ) / 4) : (kPhiP - kPhiQ);

    std::array<int32_t, 8> out_arr{};
    PhoenixInterferenceState worst_state = PhoenixInterferenceState::Bound;

    for (size_t ax = 0; ax < 8; ++ax) {
      const int64_t pred = locked_pulses_[ax] + vel_pulses_[ax];
      const int64_t err = static_cast<int64_t>(raw[ax]) - pred;
      locked_pulses_[ax] = pred + (err * alpha_num) / kPhiP;
      vel_pulses_[ax] += ((raw_vel[ax] - vel_pulses_[ax]) * beta_num) / kPhiP;
      out_arr[ax] = static_cast<int32_t>(locked_pulses_[ax]);

      const int32_t phase_err_q32 = static_cast<int32_t>(
        pulse_to_phase_q32(static_cast<int32_t>(err)) );
      const auto st = classify_phoenix_interference(phase_err_q32);
      if (static_cast<uint8_t>(st) > static_cast<uint8_t>(worst_state)) {
        worst_state = st;
      }
    }

    return StepResult{array_to_pulse(out_arr), coherence_q8, worst_state, jitter_clipped};
  }

private:
  int64_t jitter_ref_pulses_{64};
  bool initialized_{false};
  bool has_velocity_{false};
  std::array<int64_t, 8> locked_pulses_{};
  std::array<int64_t, 8> vel_pulses_{};
  std::array<int64_t, 8> prev_raw_pulses_{};
  std::array<int64_t, 8> prev_raw_vel_{};
  std::array<uint64_t, 8> jitter_ring_{};
  size_t jitter_idx_{0};
  size_t jitter_count_{0};
};

[[nodiscard]] inline int64_t eval_catmull_rom_pq(
  int64_t p0, int64_t p1, int64_t p2, int64_t p3, int64_t k, int64_t span) noexcept
{
  if (span <= 0) return p1;
  const int64_t s2 = span * span;
  const int64_t s3 = s2 * span;
  const int64_t k2 = k * k;
  const int64_t k3 = k2 * k;

  const int64_t a0 = 2 * p1;
  const int64_t a1 = -p0 + p2;
  const int64_t a2 = 2 * p0 - 5 * p1 + 4 * p2 - p3;
  const int64_t a3 = -p0 + 3 * p1 - 3 * p2 + p3;

  const int64_t num = a0 * s3 + a1 * k * s2 + a2 * k2 * span + a3 * k3;
  const int64_t den = 2 * s3;
  if (num >= 0) return (num + den / 2) / den;
  return (num - den / 2) / den;
}

/**
 * @brief Restores dropped 8-axis MELFA `PULSE` UDP packets (`valid_mask[i] == false`)
 * using exact 64-bit integer Bessel-Overhauser non-uniform Catmull-Rom cubic Hermite splines.
 */
inline size_t restore_dropped_mxt_pulses_catmull_rom_pq(
  std::vector<PULSE> & trajectory, const std::vector<bool> & valid_mask)
{
  const size_t n = trajectory.size();
  if (n <= 1 || valid_mask.size() != n) return 0;

  std::vector<size_t> valid_indices;
  valid_indices.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    if (valid_mask[i]) valid_indices.push_back(i);
  }
  if (valid_indices.empty() || valid_indices.size() == n) return 0;

  size_t restored = 0;
  size_t pos = 0;
  for (size_t i = 0; i < n; ++i) {
    if (valid_mask[i]) {
      while (pos + 1 < valid_indices.size() && valid_indices[pos + 1] <= i) ++pos;
      continue;
    }
    if (i < valid_indices.front()) {
      trajectory[i] = trajectory[valid_indices.front()];
      ++restored;
      continue;
    }
    if (i > valid_indices.back()) {
      trajectory[i] = trajectory[valid_indices.back()];
      ++restored;
      continue;
    }

    while (pos + 1 < valid_indices.size() && valid_indices[pos + 1] < i) ++pos;
    const size_t k1 = pos;
    const size_t k2 = (pos + 1 < valid_indices.size()) ? (pos + 1) : pos;
    const size_t k0 = (k1 > 0) ? (k1 - 1) : k1;
    const size_t k3 = (k2 + 1 < valid_indices.size()) ? (k2 + 1) : k2;

    const int64_t idx0 = static_cast<int64_t>(valid_indices[k0]);
    const int64_t idx1 = static_cast<int64_t>(valid_indices[k1]);
    const int64_t idx2 = static_cast<int64_t>(valid_indices[k2]);
    const int64_t idx3 = static_cast<int64_t>(valid_indices[k3]);

    const int64_t span = idx2 - idx1;
    const int64_t step_k = static_cast<int64_t>(i) - idx1;
    const int64_t h0 = idx1 - idx0;
    const int64_t h1 = span;
    const int64_t h2 = idx3 - idx2;

    const auto y0 = pulse_to_array(trajectory[valid_indices[k0]]);
    const auto p1 = pulse_to_array(trajectory[valid_indices[k1]]);
    const auto p2 = pulse_to_array(trajectory[valid_indices[k2]]);
    const auto y3 = pulse_to_array(trajectory[valid_indices[k3]]);

    std::array<int32_t, 8> out_arr{};
    for (size_t ax = 0; ax < 8; ++ax) {
      const int64_t y0_a = y0[ax];
      const int64_t p1_a = p1[ax];
      const int64_t p2_a = p2[ax];
      const int64_t y3_a = y3[ax];

      int64_t m1 = 0;
      if (h0 > 0 && h1 > 0) {
        m1 = ((h1 * h1 * (p1_a - y0_a)) + (h0 * h0 * (p2_a - p1_a))) / (h0 * (h0 + h1));
      } else {
        m1 = p2_a - p1_a;
      }

      int64_t m2 = 0;
      if (h2 > 0 && h1 > 0) {
        m2 = (((h2 * h2 * (p2_a - p1_a)) + (h1 * h1 * (y3_a - p2_a))) * h1) /
             (h2 * h1 * (h1 + h2));
      } else {
        m2 = p2_a - p1_a;
      }

      const int64_t p0_a = p2_a - 2 * m1;
      const int64_t p3_a = p1_a + 2 * m2;
      out_arr[ax] = static_cast<int32_t>(
        eval_catmull_rom_pq(p0_a, p1_a, p2_a, p3_a, step_k, span));
    }

    trajectory[i] = array_to_pulse(out_arr);
    ++restored;
  }

  return restored;
}

}  // namespace MelfaEthernet::phoenix_pq

#endif  // __MELFA_PHOENIX_LATTICE_PQ_HPP__
