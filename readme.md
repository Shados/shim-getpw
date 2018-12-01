# shim-getpw

## What
This is a tiny `LD_PRELOAD` shim to allow overriding the results of
`getpwuid()` and `getpwnam()` with values provided from environment variables.

If you want anything at all beyond that, you should look at
[nss_wrapper](https://cwrap.org/nss_wrapper.html) and the other
[cwrap](https://cwrap.org/) libraries. I mostly only wrote this because I was
not aware of them at the time, and couldn't find an existing shim. I'm keeping
it because `nss_wrapper` is overkill for the majority of my uses.

`getpwuid_r()` and `getpwnam_r()` aren't shimmed, because I haven't had any
need for doing so.

## Why
Some software relies solely on `getpwuid()/getpwnam()` (and by extension, the
NSS mechanism) to determine things like a user's home directory (notably,
OpenSSH does this).  This can make it signficantly more difficult to make some
uses of such software *self-contained* (reproducible, portable). See example
usages below.


## Building

### [CMake](https://cmake.org/)
```bash
git clone https://github.com/Shados/shim-getpw
mkdir shim-getpw/build
cd shim-getpw/build
cmake ..
make
make test
```

### [Nix](https://nixos.org/nix/)
Nix will use CMake internally for building it, but has the advantage of
providing a sandboxed build environment, and not polluting your global paths
(`/usr`, `/bin`, `/lib`, etc.) with things you may not want (e.g. if you don't
use CMake yourself).

```bash
git clone https://github.com/Shados/shim-getpw
cd shim-getpw
nix build -f package.nix
```
The output will be in `./result`.

The downside of using Nix is that because it will build against Nix's isolated
libraries, it won't be usable without the Nix closure for it. Observe:
```
shados@dreamlogic[~/.../software/shim-getpw] λ ldd result/lib/libshim-getpw.so
        linux-vdso.so.1 (0x00007ffead3cb000)
        libdl.so.2 => /nix/store/g2yk54hifqlsjiha3szr4q3ccmdzyrdv-glibc-2.27/lib/libdl.so.2 (0x00007fb8936c0000)
        libc.so.6 => /nix/store/g2yk54hifqlsjiha3szr4q3ccmdzyrdv-glibc-2.27/lib/libc.so.6 (0x00007fb893308000)
        /nix/store/g2yk54hifqlsjiha3szr4q3ccmdzyrdv-glibc-2.27/lib64/ld-linux-x86-64.so.2 (0x00007fb893ad0000)
```
You could 'fix' this with use of Nix's
[PatchELF](https://nixos.org/patchelf.html) utility if for some reason you
wanted to build it this way and later distribute and use it on systems without Nix.

## Usage Examples

### Self-Contained SSH Client Configuration
```bash
cd deployment-project
mkdir .ssh
cat <<EOF > .ssh/config
IdentityFile ~/.ssh/deployment-project.id_rsa
Host example.com
    User deployment-user
    Port 3887
EOF
ssh-keyscan -p 3887 example.com > .ssh/known_hosts # Note: Check the ssh host key fingerprint matches

LD_PRELOAD=$PATH_TO_SHIM_DIR/libshim-getpw.so SHIM_HOME=$PWD ssh example.com
```
Some things to note about OpenSSH `ssh`'s configuration file:
- It does not support relative paths
- It does not respect the `HOME` environment variable
- It resolves `~` to the `pw_dir` member of the `passwd` struct returned by `getpwuid()`

Of course, you can directly point `ssh` to a configuration file with the `-F`
flag, but you're still stuck using absolute paths in that configuration file.
Overriding the value of `~` it uses is a work-around for this.

Additionally if, say, you are using a deployment, orchestration, or
configuration management tool that executes OpenSSH's `ssh` binary in order to
issue commands on remote systems, then that tool may not actually support
passing through a configuration file, and may only support a subset of the
things you need to configure in order to get a successful connection.

In this case, keeping everything (or almost everything) required to
authenticate and connect to the target systems in the same place as the
configuration for those targets also helps keep the two in sync.

### Testing Software in a Restricted Environment
This is more or less why the Samba project developed cwrap, incidentally.

As an example, you [cannot successfully run some of libssh2's integration tests inside
of a Nix build sandbox](https://github.com/NixOS/nix/issues/2007), because the
OpenSSH `sshd` verifies that the user's shell (from the `pw_shell` member of
the `passwd` struct returned by `getpwnam()`) actually exists, and in the
restricted sandbox it does not, nor can you manipulate the `passwd` database to
set it.

Using this shim, we can build libssh2 and run these tests successfully using
the below Nix expression file, via `nix-build libssh2-test.nix`

`libssh2-test.nix`:
```nix
let
  pkgs =  import <nixpkgs> { };
  shim = pkgs.callPackage ./path/to/shim-getpw { };
in

pkgs.libssh2.overrideAttrs (oldAttrs: {
  doCheck = true;
  preConfigure = ''
    # gives `sshd -dd` output for proper debugging of integration test failure
    export DEBUG=true

    # configure the sshd integration test to run correctly
    export USER=$(id -un)
    export ac_cv_path_SSHD=${pkgs.openssh}/bin/sshd
    ./buildconf
  '';

  preCheck = ''
    # shim getpwnam() so that sshd will retrieve a valid shell for the build
    # user
    export LD_PRELOAD=${shim}/lib/libshim-getpw.so
    export SHIM_SHELL=${pkgs.bash}/bin/bash
  '';
  nativeBuildInputs = with pkgs; [
    autoconf automake libtool which
  ];
})
```
