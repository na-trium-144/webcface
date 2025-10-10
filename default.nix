{
  pkgs ? import <nixpkgs> {},
  doCheck ? false,
  webui_version ? "1.15.0",
}:
let
  doCheckArg = doCheck;

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
pkgs.stdenv.mkDerivation rec {
  pname = "webcface";
  version = "3.1.1";

  srcs = [
    (builtins.path {
      path = ./.;
      name = pname;
    })
    (builtins.fetchTarball {
      url = "https://github.com/na-trium-144/webcface-webui/releases/download/v${webui_version}/webcface-webui_${webui_version}.tar.gz";
      sha256 = "sha256:0kvh3jszzp1jan3xxgxk81yyvqw8vgv10zrycq0ikyhmmzrsr2pq";
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
    crow
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
