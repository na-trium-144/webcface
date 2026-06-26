{
  pkgs ? import <nixpkgs> {},
  doCheck ? false,
  webui_version ? "1.16.0",
}:
let
  doCheckArg = doCheck;

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
pkgs.stdenv.mkDerivation rec {
  pname = "webcface";
  version = "3.3.1";

  srcs = [
    (builtins.path {
      path = ./.;
      name = pname;
    })
    (builtins.fetchTarball {
      url = "https://github.com/na-trium-144/webcface-webui/releases/download/v${webui_version}/webcface-webui_${webui_version}.tar.gz";
      sha256 = "sha256:1k38dw8147ppxrsmmx65cm71rqwfksmjj3j7faw9s8726iiw8f3y";
      name = "webcface-webui";
    })
  ];
  sourceRoot = pname;

  nativeBuildInputs = [
    pkgs.meson
    pkgs.ninja
    pkgs.pkg-config
    pkgs.cmake
    pkgs.python3Minimal
  ];
  buildInputs = [
    pkgs.gcc
    pkgs.asio
    pkgs.cli11
    pkgs.crow
    curl
    pkgs.fmt_11
    pkgs.gbenchmark
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
  
  doCheck = doCheckArg;

  postInstall = ''
    mkdir -p $out/share/webcface/
    cp -r ../../webcface-webui $out/share/webcface/dist
  '';
}
