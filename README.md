# Pol-IPd

[![C/C++ CI](https://github.com/lstasi/polipd/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/lstasi/polipd/actions/workflows/c-cpp.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

**Pol-IPd** is a Linux traffic-control daemon for NAT/router boxes. It configures the kernel's HTB (Hierarchical Token Bucket) queuing disciplines via `tc` (iproute2), enabling per-client and per-group bandwidth limits. A companion statistics service broadcasts live per-client throughput over multicast UDP, and a text-mode configuration wizard (`polipconfig`) makes it easy to create and edit the configuration file.

---

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Source Architecture](#source-architecture)
  - [Module Overview](#module-overview)
  - [Data Structures](#data-structures)
  - [Call Flow](#call-flow)
- [Building](#building)
- [Installation](#installation)
- [Configuration](#configuration)
  - [File Format](#file-format)
  - [Rate Units](#rate-units)
  - [Sample Configuration](#sample-configuration)
- [Usage](#usage)
  - [polipd](#polipd-daemon)
  - [polipconfig](#polipconfig-wizard)
- [Statistics Protocol](#statistics-protocol)
- [Init Scripts](#init-scripts)
- [Project Layout](#project-layout)
- [License](#license)

---

## Features

- Per-client and per-group bandwidth shaping using Linux HTB queuing disciplines.
- Hierarchical policies — clients can belong to groups, groups can borrow unused bandwidth.
- Live statistics broadcast over multicast UDP so remote clients can query current throughput.
- Text-mode configuration wizard (`polipconfig`) built with the [newt](https://pagure.io/newt) library.
- Clean shutdown: removes all `tc` qdiscs on `SIGTERM`/`SIGINT`.
- SFQ (Stochastic Fairness Queuing) per leaf class for fair intra-class scheduling.

---

## Requirements

| Requirement | Notes |
|---|---|
| Linux kernel 2.4+ | Needed for advanced routing / traffic control. Kernel 2.4.20+ requires no extra patches. |
| HTB3 queueing discipline | If your kernel is older than 2.4.20 you must apply the HTB3 patch from <http://luxik.cdi.cz/~devik/qos/htb/>. The patch for iproute2 is bundled in the HTB3 tarball. |
| iproute2 (`tc`) | Used at runtime to program the kernel. |
| popt | Command-line option parsing library, required by `polipd`. |
| newt + slang | Text UI library, required only by `polipconfig`. Available on most distributions via `newt-devel` and `slang-devel`. |
| Multicast route | A multicast route must be added before starting the daemon (see [Installation](#installation)). |

---

## Source Architecture

### Module Overview

```
polipd/
├── polip.h           # Shared header: includes, data structures, function prototypes
├── polipd.c          # Daemon entry point: CLI parsing, daemonisation, signal handling
├── setup.c           # Kernel setup: programs HTB qdiscs and filters via `tc`
├── server.c          # Statistics server: multicast UDP listener/broadcaster
├── stats.c           # Statistics parser: reads `tc -s class show` output
├── readconf.c        # Configuration reader: parses /etc/polip.conf
├── writeconf.c       # Configuration writer: writes /etc/polip.conf
├── poliputil.c       # Utility functions: rate conversion, IP address lookup, string helpers
├── polipconfig.c     # Interactive TUI wizard (newt) for creating/editing polip.conf
├── check.s           # x86 assembly debug checkpoints (used in debug builds only)
├── Makefile          # Build rules
├── config/
│   └── polip.conf.sample   # Annotated sample configuration file
└── init.d-scripts/
    ├── polipd        # Red Hat / chkconfig init script
    ├── redhat-02     # Alternative Red Hat init script
    └── generic       # Distribution-agnostic init script
```

### Data Structures

All shared types are defined in `polip.h`.

#### `struct root`
Represents the root network configuration. There is exactly one root per configuration file.

| Field | Type | Description |
|---|---|---|
| `netdevice` | `char[20]` | LAN-facing network interface (e.g. `eth1`) |
| `linkdevice` | `char[20]` | WAN/internet-facing interface (e.g. `eth0`) |
| `netspeed` | `char[20]` | Total LAN interface speed (e.g. `1gbit`) |
| `linkspeed` | `char[20]` | Internet link speed — the bandwidth to be shaped (e.g. `100mbit`) |
| `handle` | `int` | HTB class handle assigned during config parsing (starts at 1, increments by 3) |

#### `struct grupos` (Groups)
Represents a bandwidth group (intermediate HTB class). Multiple groups form a singly-linked list.

| Field | Type | Description |
|---|---|---|
| `name` | `char[20]` | Unique group name referenced by clients as their `parent` |
| `parent` | `char[20]` | Parent name — either `root` or another group's name |
| `rate` | `char[20]` | Guaranteed bandwidth (e.g. `10mbit`) |
| `borrow` | `char[5]` | `yes` / `no` — whether this group may borrow unused bandwidth (sets HTB `ceil`) |
| `share` | `char[5]` | Reserved for future use |
| `handle` | `int` | HTB class handle |
| `nxt` | `struct grupos *` | Pointer to next group in linked list |

#### `struct clientes` (Clients)
Represents an individual host being shaped. Multiple clients form a singly-linked list.

| Field | Type | Description |
|---|---|---|
| `name` | `char[20]` | Client name |
| `parent` | `char[20]` | Parent name — either `root` or a group name |
| `rate` | `char[20]` | Guaranteed bandwidth |
| `borrow` | `char[5]` | `yes` / `no` — whether this client may borrow unused bandwidth |
| `share` | `char[5]` | Reserved for future use |
| `ip` | `char[25]` | Client IP address — used for HTB filter matching |
| `handle` | `int` | HTB class handle |
| `speed` | `int` | Current measured throughput in bytes/sec (updated by `stats.c`) |
| `pstats` | `pid_t` | Non-zero when a remote client has requested statistics for this host |
| `tstats` | `time_t` | Timestamp of the last `Start` statistics request |
| `nxt` | `struct clientes *` | Pointer to next client in linked list |

### Call Flow

```
polipd -d
  └── fork() ──► child process
        ├── readconf()      reads /etc/polip.conf → populates root/grupos/clientes structures
        ├── setup()         calls `tc` to install HTB root qdisc, classes, SFQ leaf qdiscs,
        │                   and u32 filters for each client IP
        └── server()        enters event loop:
              ├── bind multicast socket 230.0.0.1:20002  (receive Start/Stop from clients)
              ├── select() with 1-second timeout
              ├── on Start message  → mark client pstats=1, record timestamp
              ├── on Stop  message  → clear client pstats
              └── every iteration  → stats() parses `tc -s class show` output,
                                     broadcasts updated throughput on 230.0.0.2:20002
```

**Signal handling:** `SIGTERM` and `SIGINT` trigger `ExitClean()`, which removes the root HTB qdisc from the network device before exiting. `SIGKILL` leaves the qdisc in place.

---

## Building

```bash
# Build both polipd and polipconfig
make

# Build with verbose tc command output (defines __VERBOSE__)
# Edit Makefile and add -D__VERBOSE__ to CFLAGS, then:
make

# Debug build (includes check.s checkpoints)
make debug

# Clean build artefacts
make clean
```

**Dependencies required at build time:**

```bash
# Debian / Ubuntu
sudo apt-get install libpopt-dev libnewt-dev libslang2-dev

# Red Hat / Fedora / CentOS
sudo yum install popt-devel newt-devel slang-devel
```

---

## Installation

1. **Compile** (see [Building](#building)).

2. **Install binaries:**

   ```bash
   sudo make install
   # Copies polipd and polipconfig to /usr/bin/
   ```

3. **Add the multicast route** (replace `eth1` with your LAN interface):

   ```bash
   sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev eth1
   ```

4. **Create the configuration file** — either manually (see [Configuration](#configuration)) or with the wizard:

   ```bash
   sudo polipconfig
   ```

5. **Install an init script** (choose the one matching your distribution):

   ```bash
   # Red Hat / CentOS
   sudo cp init.d-scripts/polipd /etc/init.d/polipd
   sudo chmod +x /etc/init.d/polipd
   sudo chkconfig --add polipd

   # Generic
   sudo cp init.d-scripts/generic /etc/init.d/polipd
   sudo chmod +x /etc/init.d/polipd
   ```

6. **Apply the HTB3 kernel patch** if your kernel is older than 2.4.20. The patch and instructions are available at <http://luxik.cdi.cz/~devik/qos/htb/>. The iproute2 patch is bundled in the same archive.

---

## Configuration

### File Format

The configuration file `/etc/polip.conf` uses an INI-style format with three section types: `[root]`, `[grupo]`, and `[Cliente]`. Lines beginning with `#` are comments. **Order matters** — groups and clients must appear after their parent.

#### `[root]` section (required, exactly one)

```ini
[root]
netdevice=<LAN interface>
netspeed=<total LAN speed>
linkdevice=<WAN interface>
linkspeed=<internet link speed>
```

#### `[grupo]` section (optional, repeatable)

```ini
[grupo]
name=<group name>
parent=<root | group name>
rate=<guaranteed bandwidth>
borrow=<yes | no>
```

#### `[Cliente]` section (optional, repeatable)

```ini
[Cliente]
name=<client name>
parent=<root | group name>
rate=<guaranteed bandwidth>
borrow=<yes | no>
ip=<client IPv4 address>
```

### Rate Units

| Unit | Meaning |
|---|---|
| `bps` | Bits per second |
| `kbit` | Kilobits per second (×1,024) |
| `mbit` | Megabits per second (×1,024²) |
| `kbps` | Kilobytes per second (×1,024 × 8) |
| `mbps` | Megabytes per second (×1,024² × 8) |

### Sample Configuration

```ini
#Config file for polipd
#This file must be in /etc/polip.conf

[root]
netdevice=eth1
netspeed=1gbit
linkdevice=eth0
linkspeed=100mbit

# A group that shares 10 Mbit from the internet link
[grupo]
name=Group
parent=root
rate=10mbit
borrow=yes

# Two clients attached directly to root
[Cliente]
name=clientroot1
parent=root
rate=20mbit
borrow=yes
ip=192.168.1.30

[Cliente]
name=clientroot2
parent=root
rate=50mbit
borrow=yes
ip=192.168.1.41

# Three clients inside Group (rates must not exceed the group's 10mbit total)
[Cliente]
name=clients.60
parent=Group
rate=3mbit
borrow=yes
ip=192.168.1.60

[Cliente]
name=clients.61
parent=Group
rate=3mbit
borrow=yes
ip=192.168.1.61

[Cliente]
name=clients.62
parent=Group
rate=3mbit
borrow=yes
ip=192.168.1.62
```

---

## Usage

### polipd Daemon

```
polipd [OPTION]

Options:
  -d, --daemon   Daemonise: fork to background, configure kernel, start statistics server
  -c, --clear    Remove all HTB qdiscs from the network device and exit
  -t, --test     Test mode: configure kernel and start statistics server without forking
  -h, --help     Print help
```

**Start as a daemon:**

```bash
sudo polipd -d
```

**Remove all traffic-shaping rules:**

```bash
sudo polipd -c
```

**Test the configuration without daemonising (useful for debugging):**

```bash
sudo polipd -t
```

The daemon writes all `tc` commands it executes to `/var/run/polipd` for audit and troubleshooting.

### polipconfig Wizard

`polipconfig` is an interactive text-mode wizard (built with [newt](https://pagure.io/newt)) for creating and editing `/etc/polip.conf`. It must be run as root because it writes to `/etc/polip.conf`.

```bash
sudo polipconfig
```

The wizard presents menus to:
- Configure the root device and speeds.
- Add, edit, or delete bandwidth groups.
- Add, edit, or delete clients (with name, parent, rate, borrow flag, and IP address).
- Save the configuration on exit.

---

## Statistics Protocol

The statistics server uses two multicast addresses on port **20002/UDP**:

| Direction | Multicast Address | Purpose |
|---|---|---|
| Inbound | `230.0.0.1:20002` | Remote clients send `Start` / `Stop` control messages |
| Outbound | `230.0.0.2:20002` | Daemon broadcasts per-client throughput data |

**Control messages (inbound):**

| Message | Effect |
|---|---|
| `Start` | Begin reporting statistics for the sending IP |
| `Stop` | Stop reporting statistics for the sending IP |

**Data packet format (outbound):**

```
<link_speed_bps> <ip>/<speed_bps>/<rate_bps>/ [<ip>/<speed_bps>/<rate_bps>/ ...]
```

- `link_speed_bps` — total internet link capacity in bytes/sec.
- `ip` — client IP address.
- `speed_bps` — current measured throughput in bytes/sec (from `tc -s class show`).
- `rate_bps` — configured guaranteed rate in bytes/sec.

If a client has been active for more than 10 minutes without a new `Start` message, the daemon sends a `Report` request to prompt the remote client to re-register.

---

## Init Scripts

Three init scripts are provided in `init.d-scripts/`:

| File | Description |
|---|---|
| `polipd` | Red Hat / CentOS script compatible with `chkconfig` (runlevels 3, 4, 5). |
| `redhat-02` | Alternative Red Hat script. |
| `generic` | Minimal distribution-agnostic script (`start` / `stop`). |

---

## Project Layout

```
polipd/
├── polip.h                  Shared types and function prototypes
├── polipd.c                 Entry point, CLI, signal handling
├── setup.c                  HTB kernel configuration via tc
├── server.c                 Multicast UDP statistics server
├── stats.c                  tc output parser for live throughput
├── readconf.c               /etc/polip.conf parser
├── writeconf.c              /etc/polip.conf writer
├── poliputil.c              Utility: rate conversion, IP lookup, string helpers
├── polipconfig.c            newt-based interactive configuration wizard
├── check.s                  x86 assembly debug checkpoints (debug builds only)
├── Makefile                 Build system
├── config/
│   └── polip.conf.sample    Annotated sample configuration
├── init.d-scripts/
│   ├── polipd               Red Hat init script
│   ├── redhat-02            Alternative Red Hat init script
│   └── generic              Generic init script
├── LICENSE                  GNU General Public License v3
└── README.md                This file
```

---

## License

Pol-IPd is free software released under the [GNU General Public License v3.0](LICENSE).

Copyright © Leandro Stasi and contributors.