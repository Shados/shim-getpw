{ pkgs ? import <nixpkgs> { }
}:
with pkgs;
stdenv.mkDerivation {
  name = "shim-getpwuid-env";

  buildInputs = [
    (pkgs.callPackage ./default.nix { }).buildInputs
    cppcheck
  ];
}
