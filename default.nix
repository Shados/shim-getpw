{ stdenv, cmake
, openssh
}:
stdenv.mkDerivation {
  name = "shim-getpw";

  src = ./.;

  buildInputs = [
    cmake
  ];

  doCheck = true;
}
