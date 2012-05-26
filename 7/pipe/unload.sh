#!/bin/sh
module="scull_pipe"
device="scull_pipe"
# invoke rmmod with all arguments we got
/sbin/rmmod $module $* || exit 1
# Remove stale nodes
rm -f /dev/${device} /dev/${device}[0-3]
