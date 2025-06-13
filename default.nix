{
  pkgs ? import <nixpkgs> {},
  doCheck ? false,
}:
let
  crow = pkgs.crow.overrideAttrs (oldAttrs: {
    src = pkgs.fetchFromGitHub {
      owner = "na-trium-144";
      repo = "crow";
      rev = "11bf7a0dfacc1df9f2fbfa5838828817ff58661b";
      sha256 = "sha256-RylTWJoOLBCuOQWpuIO14ssUd6AstEV6AbwF/7OdIe0=";
    };
    cmakeFlags = [
      (pkgs.lib.cmakeBool "CROW_BUILD_EXAMPLES" false)
      (pkgs.lib.cmakeBool "CROW_BUILD_TESTS" false)
    ];
  });
  curl = pkgs.curl.overrideAttrs (oldAttrs: {
    configureFlags = oldAttrs.configureFlags ++ [
      (pkgs.lib.enableFeature true "websockets")
    ];
  });
  spdlog = pkgs.spdlog.overrideAttrs (oldAttrs: {
    buildInputs = oldAttrs.buildInputs ++ [ pkgs.fmt_11 ];
    propagatedBuildInputs = [ pkgs.fmt_11 ];
  });
in
pkgs.stdenv.mkDerivation {
  pname = "webcface";
  version = "2.9.0";

  src = ./.;

  nativeBuildInputs = [
    pkgs.meson
    pkgs.ninja
    pkgs.pkg-config
    pkgs.cmake
  ];
  buildInputs = [
    pkgs.gcc
    pkgs.asio
    pkgs.cli11
    crow
    curl
    pkgs.fmt_11
    pkgs.gtest
    pkgs.vips
    pkgs.msgpack-cxx
    spdlog
    pkgs.utf8cpp
  ];
  propagatedBuildInputs = [
    curl
    pkgs.fmt_11
    pkgs.vips
    spdlog
  ];

  mesonFlags = ["-Ddownload_webui=disabled"]
    ++ (if doCheck then [ "-Dtests=enabled" ] else [ "-Dtests=disabled" ]);
  doCheck = doCheck;
}
