#!/bin/sh

evtest /dev/input/event2 | ./evtest_key_filter.pl | ./build/input_overlay
