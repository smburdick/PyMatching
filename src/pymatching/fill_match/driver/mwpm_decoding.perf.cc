#include "pymatching/fill_match/driver/mwpm_decoding.h"

#include "pymatching/perf/util.perf.h"
#include "stim.h"

std::pair<stim::DetectorErrorModel, std::vector<stim::SparseShot>> generate_data(
    size_t distance, size_t rounds, double noise, size_t num_shots) {
    stim::CircuitGenParameters gen(rounds, distance, "rotated_memory_x");
    gen.after_clifford_depolarization = noise;
    gen.after_reset_flip_probability = noise;
    gen.before_measure_flip_probability = noise;
    stim::Circuit circuit = stim::generate_surface_code_circuit(gen).circuit;
    std::mt19937_64 rng(0);  // NOLINT(cert-msc51-cpp)
    size_t num_detectors = circuit.count_detectors();
    auto results = stim::detector_samples(circuit, num_shots, false, true, rng);
    std::vector<stim::SparseShot> shots;
    for (size_t k = 0; k < num_shots; k++) {
        shots.push_back({});
        shots.back().obs_mask = results[num_detectors][k];
        for (size_t d = 0; d < num_detectors; d++) {
            if (results[d][k]) {
                shots.back().hits.push_back(d);
            }
        }
    }

    auto dem = stim::ErrorAnalyzer::circuit_to_detector_error_model(circuit, false, true, false, 0, false, false);

    return {dem, shots};
}

BENCHMARK(Decode_surface_r5_d5_p1000) {
    size_t rounds = 5;
    auto data = generate_data(5, rounds, 0.001, 1024);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_micros(270)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r11_d11_p100) {
    size_t rounds = 11;
    auto data = generate_data(11, rounds, 0.01, 128);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(10)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r11_d11_p1000) {
    size_t rounds = 11;
    auto data = generate_data(11, rounds, 0.001, 512);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(1.4)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r11_d11_p10000) {
    size_t rounds = 11;
    auto data = generate_data(11, rounds, 0.0001, 256);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_micros(80)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r11_d11_p100000) {
    size_t rounds = 11;
    auto data = generate_data(11, rounds, 0.00001, 1024);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_micros(31)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p100) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.01, 8);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(6.8)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == 0) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p100_max_buckets) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.01, 8);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1 << (sizeof(pm::weight_int) * 8 - 8);
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(7.0)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == 0) {
        std::cerr << "data dependence";
    }
}


BENCHMARK(Decode_surface_r21_d21_p100_with_dijkstra) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.01, 8);
    auto &dem = data.first;
    // Add fake observable > 64 to trigger general decoder with Dijkstra post-processing
    dem.append_logical_observable_instruction(stim::DemTarget::observable_id(128));
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }
    size_t num_mistakes = 0;
    pm::ExtendedMatchingResult res(mwpm.flooder.graph.num_observables);
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            pm::decode_detection_events(mwpm, shot.hits, res.obs_crossed.data(), res.weight);
            if (shot.obs_mask != res.obs_crossed[0]) {
                num_mistakes++;
            }
            res.reset();
        }
    })
        .goal_millis(7.7)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == 0) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p1000) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.001, 256);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(5.6)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}


BENCHMARK(Decode_surface_r21_d21_p1000_max_buckets) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.001, 256);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1 << (sizeof(pm::weight_int) * 8 - 8);
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(5.6)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p1000_with_dijkstra) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.001, 256);
    auto &dem = data.first;
    // Add fake observable > 64 to trigger general decoder with Dijkstra post-processing
    dem.append_logical_observable_instruction(stim::DemTarget::observable_id(128));
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    pm::ExtendedMatchingResult res(mwpm.flooder.graph.num_observables);
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            pm::decode_detection_events(mwpm, shot.hits, res.obs_crossed.data(), res.weight);
            if (shot.obs_mask != res.obs_crossed[0]) {
                num_mistakes++;
            }
            res.reset();
        }
    })
        .goal_millis(7.6)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p10000) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.0001, 512);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(0.930)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p10000_max_buckets) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.0001, 512);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1 << (sizeof(pm::weight_int) * 8 - 8);
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_millis(0.950)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p10000_with_dijkstra) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.0001, 512);
    auto &dem = data.first;
    // Add fake observable > 64 to trigger general decoder with Dijkstra post-processing
    dem.append_logical_observable_instruction(stim::DemTarget::observable_id(128));
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    pm::ExtendedMatchingResult res(mwpm.flooder.graph.num_observables);
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            pm::decode_detection_events(mwpm, shot.hits, res.obs_crossed.data(), res.weight);
            if (shot.obs_mask != res.obs_crossed[0]) {
                num_mistakes++;
            }
            res.reset();
        }
    })
        .goal_millis(1.3)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p100000) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.00001, 512);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_micros(90)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p100000_max_buckets) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.00001, 512);
    const auto &dem = data.first;
    const auto &shots = data.second;

    size_t num_buckets = 1 << (sizeof(pm::weight_int) * 8 - 8);
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            auto res = pm::decode_detection_events_for_up_to_64_observables(mwpm, shot.hits);
            if (shot.obs_mask != res.obs_mask) {
                num_mistakes++;
            }
        }
    })
        .goal_micros(92)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}

BENCHMARK(Decode_surface_r21_d21_p100000_with_dijkstra) {
    size_t rounds = 21;
    auto data = generate_data(21, rounds, 0.00001, 512);
    auto &dem = data.first;
    // Add fake observable > 64 to trigger general decoder with Dijkstra post-processing
    dem.append_logical_observable_instruction(stim::DemTarget::observable_id(128));
    const auto &shots = data.second;

    size_t num_buckets = 1024;
    auto mwpm = pm::detector_error_model_to_mwpm(dem, num_buckets);

    size_t num_dets = 0;
    for (const auto &shot : shots) {
        num_dets += shot.hits.size();
    }

    size_t num_mistakes = 0;
    pm::ExtendedMatchingResult res(mwpm.flooder.graph.num_observables);
    benchmark_go([&]() {
        for (const auto &shot : shots) {
            pm::decode_detection_events(mwpm, shot.hits, res.obs_crossed.data(), res.weight);
            if (shot.obs_mask != res.obs_crossed[0]) {
                num_mistakes++;
            }
            res.reset();
        }
    })
        .goal_micros(120)
        .show_rate("dets", (double)num_dets)
        .show_rate("layers", (double)rounds * shots.size())
        .show_rate("shots", (double)shots.size());
    if (num_mistakes == shots.size()) {
        std::cerr << "data dependence";
    }
}