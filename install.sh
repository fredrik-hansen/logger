#!/bin/sh

# Installation script for logger (for system wide installs)
# Run as root!

# Use LIBDIR from environment, default to /lib
LIBDIR="${LIBDIR:-/lib}"
LOGGER_SO="${LIBDIR}/logger.so"

if [ ! -x "$LOGGER_SO" ]; then
    echo "Failed: $LOGGER_SO not found or not executable"
    echo "Are you installing this as root?"
    exit 1
fi

touch /etc/ld.so.preload || {
    echo "Failed: cannot create /etc/ld.so.preload"
    echo "Are you installing this as root?"
    exit 1
}

if [ ! -w /etc/ld.so.preload ]; then
    echo "Failed: /etc/ld.so.preload is not writable"
    echo "Are you installing this as root?"
    exit 1
fi

# Check if logger.so already in preload file (with proper escaping)
if grep -qF "$LOGGER_SO" /etc/ld.so.preload; then
    echo "logger already installed... updated to newest version"
else
    echo "$LOGGER_SO" >> /etc/ld.so.preload
    echo "Installed to $LOGGER_SO"
fi
