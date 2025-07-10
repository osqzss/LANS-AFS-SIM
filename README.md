# LANS-AFS-SIM

LANS-AFS-SIM is a baseband waveform generator for the Lunar Augmented Navigation Service (LANS), developed to support future algorithm research and testing of LANS Augmented Forward Signal (AFS) receivers. The generated baseband signal can be processed directly by a software-defined receiver or converted to RF using an SDR platform, such as bladeRF or LimeSDR.

LANS is the lunar equivalent of GNSS, designed for future lunar explorations. The AFS is a standardized signal structure for LANS, with its recommended standard available at NASA's [LunaNet Interoperability Specification](https://www.nasa.gov/directorates/somd/space-communications-navigation-program/lunanet-interoperability-specification/) site. The AFS is transmitted by multiple LunaNet Service Provider (LNSP) nodes, including NASA's Lunar Communications Relay and Navigation Systems (LCRNS), ESA's Moonlight Lunar Communications and Navigation Services (LCNS), and JAXA's Luna Navigation Satellite System (LNSS).

Together with a software-defined LANS AFS receiver, such as [PocketSDR-AFS](https://github.com/osqzss/PocketSDR-AFS), LANS-AFS-SIM provides a valuable platform for validating signal processing algorithms and assessing navigation performance.

## Features

- Generates LANS AFS baseband signals for research and testing.
- Supports output in 2-bit I/Q emulating typical GNSS RF frontends.
- Compatible with SDR hardware platforms (e.g., bladeRF, LimeSDR).
- Designed for integration with software-defined receivers, especially PocketSDR-AFS.

## Build Instructions

Clone the repository and build using `make`:

```sh
git clone https://github.com/osqzss/LANS-AFS-SIM.git
cd LANS-AFS-SIM
make
```

## Usage

### Generating a 16-bit I/Q Baseband Signal

By default, LANS-AFS-SIM generates a baseband signal file in 16-bit complex (interleaved short I/Q) format at a 12MHz sampling rate, suitable for conversion to RF via an SDR platform:

```sh
./afs_sim -t 90 afssim_iq16.bin
```

### Generating a 2-bit I/Q Sample File

To generate a 2-bit I/Q sample file (emulating a general GNSS RF frontend output with additive Gaussian noise), use:

```sh
./afs_sim -t 90 -b 2 afssim_iq2.bin
```

This format can be fed directly into PocketSDR-AFS for offline receiver testing.

## Notes

- For development and testing purposes, the default center frequency of the simulator is set to 1575.42MHz. Although the actual LANS AFS is broadcast in the S-band, this configuration allows the use of readily available L-band frontend devices (such as FE2CH) for real-time testing with PocketSDR-AFS.
- To configure the simulator for the actual S-band transmission frequency of LANS AFS, open the `afs_sim.c` file and comment out the `#define DEMO_L1` directive. This change switches the center frequency to the S-band, matching the authentic LANS AFS broadcast.

## References

For additional details on the architecture and examples of off-line and real-time test results, please refer to our publication:

Sobukawa, R., & Ebinuma, T. (2025). Open-Source Real-Time SDR Platform for Rapid Prototyping of LANS AFS Receiver. Aerospace, 12(7), 620. [https://doi.org/10.3390/aerospace12070620](https://www.mdpi.com/2226-4310/12/7/620)
