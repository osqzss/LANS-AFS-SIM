# LANS-AFS-SIM

LANS-AFS-SIM is a baseband waveform generator for the Lunar Augmented Navigation Service (LANS), developed to support future algorithm research and testing of LANS Augmented Forward Signal (AFS) receivers. The generated baseband signal can be processed directly by a software-defined receiver or converted to RF using an SDR platform, such as bladeRF or LimeSDR.

LANS is the lunar equivalent of GNSS, designed for future lunar explorations. The AFS is a standardized signal structure for LANS, with its recommended standard available at NASA's [LunaNet Interoperability Specification](https://www.nasa.gov/directorates/somd/space-communications-navigation-program/lunanet-interoperability-specification/) site.

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

### Notes

- For testing purposes, the center frequency of the baseband signal is set to 1575.42MHz. Although the actual LANS AFS is broadcast in S-band, this configuration allows the use of an L-band frontend device (such as FE2CH) for real-time testing with PocketSDR-AFS.
