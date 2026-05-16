# LANS-AFS-SIM with Multipath Extension

This version extends LANS-AFS-SIM by adding lunar-surface reflected and knife-edge diffracted propagation paths for more realistic evaluation of LunaNet AFS receiver performance in lunar surface environments.

## Usage

```sh
./afs_sim [options] output_file
```

To write the binary I/Q stream to standard output, use `-` as `output_file`.

```sh
./afs_sim [options] -
```

## Command-line options

| Option | Description | Default |
|---|---|---|
| `-t tsec` | Simulation duration in seconds. | `300` |
| `-s freq` | Sampling frequency. Values smaller than `1.0e6` are interpreted in MHz; for example, `-s 2.6` means 2.6 MHz. | `12.0e6` |
| `-e feph` | Almanac file. | `default_almanac.txt` |
| `-b bits` | Output quantization bits. Currently, this option accepts only `2`. Without this option, 16-bit I/Q samples are generated. | 16-bit I/Q |
| `-l lat:lon:hgt` | User position in latitude, longitude, and height. Latitude and longitude are in degrees, and height is in meters. | Shackleton crater: `-89.67:129.78:-2786.0` |
| `-reflection` | Enable the lunar-surface reflected path. | Disabled |
| `-diffraction` | Enable the knife-edge diffraction path. | Disabled |
| `-log logfile` | Write multipath-related values to a CSV log file. | Disabled |

## Output format

The output file contains interleaved baseband I/Q samples. 

By default, the simulator generates 16-bit I/Q samples. This format is intended for driving an SDR device when the simulated AFS is transmitted as an RF signal and then received by real receiver hardware.

When `-b 2` is specified, the simulator generates 2-bit quantized I/Q samples. This format emulates the I/Q output of a typical GNSS RF frontend and can be directly interfaced with a software receiver, such as [PocketSDR-AFS](https://github.com/osqzss/PocketSDR-AFS).

## Examples

```sh
./afs_sim -t 300 -s 2.6 -e multipath/multipath_almanac.txt -log log.txt -reflection -diffraction afssim_iq16.bin
```
This example generates 300 seconds of 16-bit I/Q samples at a sampling rate of 2.6 MHz using the almanac file `multipath/multipath_almanac.txt`. Both the reflection and diffraction models are enabled, and the multipath parameters computed during the simulation are written to `log.txt`. The generated I/Q samples are saved to `afssim_iq16.bin`.

## Multipath model parameters

The main command-line options only enable or disable the reflection and diffraction models. The physical parameters used by these models are currently hard-coded in `multipath/multipath.h`.

Examples of hard-coded parameters include:

- antenna height
- complex relative permittivity of the lunar regolith
- lunar surface RMS roughness
- antenna gain scaling factor for the reflected path
- diffraction elevation mask
- knife-edge distance

Edit `multipath/multipath.h` and rebuild the program to change these parameters.

## Notes

- The simulator automatically selects the satellites visible above the local horizon at the receiver position.
- The simulation start time is taken from the first valid almanac entry.
- The reflected path and diffracted path can be enabled independently.

