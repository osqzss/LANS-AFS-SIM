#! /bin/bash

limeConfig --initialize --samplerate=12e6 --txen=1 --txlo=1575.42e6 --txlpf=12e6 --txpath=Band1 --txgain=10 --log=verbose
# for LimeSDR mini 2.0 with 20dB external attenuation
