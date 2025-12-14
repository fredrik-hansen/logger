# logger

LD_PRELOAD library that intercepts `execv()` and `execve()` system calls and logs all executed commands to syslog.

## Features

- Logs all process executions to syslog (`LOG_AUTHPRIV` facility)
- Configurable filtering by UID
- Disk space checking to prevent crashes
- System-wide or per-process installation

## Requirements

- Linux system with glibc
- gcc compiler
- Root access for system-wide installation

## Build

```bash
make
```

## Installation

### System-wide (affects all processes)

```bash
sudo make install
```

Default install location: `/lib/logger.so`

Custom install location:

```bash
sudo make install PREFIX=/usr/local LIBDIR=/usr/local/lib
```

### Per-process (manual LD_PRELOAD)

```bash
LD_PRELOAD=./logger.so command args...
```

Or use the provided wrapper:

```bash
./start command args...
```

## Testing

Verify logger is loaded:

```bash
make test
./detect
```

## Uninstall

```bash
sudo make uninstall
```

## Configuration

Edit `logger.c` before compilation:

- `UID0_ONLY`: Only log root actions (1=enabled, 0=disabled)
- `NO_SYSTEM`: Skip UIDs below this value (e.g., 500 to skip service accounts)
- `MAX`: Maximum size of logged arguments (0=unlimited, slower)
- `MIN_DISK_SPACE_KB`: Minimum disk space required for logging (default: 1MB)
- `MAX_ARGC`: Maximum number of arguments to process (default: 1024)

## Viewing Logs

```bash
# View logs (location varies by distro)
sudo tail -f /var/log/secure      # RHEL/CentOS
sudo tail -f /var/log/auth.log    # Debian/Ubuntu

# Or use journalctl
sudo journalctl -t logger -f
```

## Security Considerations

- **Bypassable**: Can be circumvented with `exec /lib64/ld-linux.so /bin/bash`
- **System-wide impact**: Affects ALL processes when installed via ld.so.preload
- **Performance**: Adds overhead to every process execution
- **Privacy**: Logs may contain sensitive command-line arguments

Use for auditing/debugging, not as a security boundary.

## License

GNU General Public License v2 or later. See source files for details.
