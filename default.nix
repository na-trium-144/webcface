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
      rev = "efb9d8c2c6260cc74a04e7be75a167d7863479b5";
      sha256 = "sha256-2foLrHG0PqamYp2/j3DbcaKdorS2p/SQALBZNYC42/M=";
    };
    postPatch = ''
      echo "# dummy CPM.cmake to avoid SSL error" > cmake/CPM.cmake
    '';
    patches = [];
    cmakeFlags = [
      (pkgs.lib.cmakeBool "CROW_BUILD_EXAMPLES" false)
      (pkgs.lib.cmakeBool "CROW_BUILD_TESTS" false)
      (pkgs.lib.cmakeBool "CROW_GENERATE_SBOM" false)
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
  version = "3.2.1";

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
